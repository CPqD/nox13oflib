#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include "discovery.hh"
#include "assert.hh"
#include "component.hh"
#include "nox.hh"
#include "packets.h"
#include "vlog.hh"
#include "timeval.hh"
#include "netinet++/ethernet.hh"
#include "openflow/openflow.h"

#include "datapath-join.hh"
#include "datapath-leave.hh"
#include "ofp-msg-event.hh"

#include "../../../oflib/ofl-structs.h"
#include "../../../oflib/ofl-messages.h"

#define PER_PORT_ARG   "per_port"
#define PACKET_OUT_ARG "pkt_out"
#define TIMEOUT_ARG    "check"
#define LINK_TO_ARG    "link_to"

namespace {

using namespace vigil;
using namespace vigil::container;

Vlog_module lg("discovery");

Discovery::Discovery(const Context* c, const json_object*) : Component(c),
    dps(), ports(), lldps(), links(), next_port(-1),
    per_port(PER_PORT_PERIOD),  packet_out(PACKET_OUT_PERIOD),
    timeout(TIMEOUT_CHECK_PERIOD), link_timeout(LINK_TIMEOUT),
    per_port_tv(), timeout_tv() { }

void
Discovery::configure(const Configuration* c) {
    hash_map<std::string, std::string> args = c->get_arguments_list();
    hash_map<std::string, std::string>::iterator iter;
    if ((iter = args.find(PER_PORT_ARG)) != args.end()) {
        per_port = atoi(iter->second.c_str());
    }
    if ((iter = args.find(PACKET_OUT_ARG)) != args.end()) {
        packet_out = atoi(iter->second.c_str());
    }
    if ((iter = args.find(TIMEOUT_ARG)) != args.end()) {
        timeout = atoi(iter->second.c_str());
    }
    if ((iter = args.find(LINK_TO_ARG)) != args.end()) {
        link_timeout = atoi(iter->second.c_str());
    }

    register_event(Link_event::static_get_name());

    timeout_tv.tv_sec  =  timeout / 1000;
    timeout_tv.tv_usec = (timeout % 1000) * 1000;

    update_timeval();
}

void
Discovery::install() {
    register_handler(Datapath_join_event::static_get_name(), boost::bind(&Discovery::dp_join_handler, this, _1));
    register_handler(Datapath_leave_event::static_get_name(), boost::bind(&Discovery::dp_leave_handler, this, _1));
    register_handler(Ofp_msg_event::get_name(OFPT_PORT_STATUS), boost::bind(&Discovery::port_status_handler, this, _1));
    register_handler(Ofp_msg_event::get_name(OFPT_PACKET_IN), boost::bind(&Discovery::packet_in_handler, this, _1));

    post(boost::bind(&Discovery::send_lldp, this));
    post(boost::bind(&Discovery::timeout_links, this));
}

Disposition
Discovery::dp_join_handler(const Event& e) {
    const Datapath_join_event& dpj = assert_cast<const Datapath_join_event&>(e);

    if (dpj.dpid.as_host() > 0x0000ffffffffffffULL) {
        lg.warn("Joined DPID has no 0x0000 in upper 16-bits, so Discovery will not work correctly");
    }

    dp_add((struct ofl_msg_features_reply *)**dpj.msg);
    return CONTINUE;
}

Disposition
Discovery::dp_leave_handler(const Event& e) {
    const Datapath_leave_event& dpe = assert_cast<const Datapath_leave_event&>(e);

    dp_remove(dpe.datapath_id);
    return CONTINUE;
}

Disposition
Discovery::port_status_handler(const Event& e) {
    const Ofp_msg_event& ome = assert_cast<const Ofp_msg_event&>(e);

    struct ofl_msg_port_status *status = (struct ofl_msg_port_status *)**ome.msg;

    // sane ports only
    if (status->desc->port_no <= OFPP_MAX) {
        if (status->reason == OFPPR_ADD) {
            port_add(Port(ome.dpid, status->desc->port_no));
        } else if (status->reason == OFPPR_DELETE) {
            port_delete(Port(ome.dpid, status->desc->port_no), Link_event::PORT);
        }
    }

    return CONTINUE;
}

Disposition
Discovery::packet_in_handler(const Event& e) {
    const Ofp_msg_event& ome = assert_cast<const Ofp_msg_event&>(e);

    struct ofl_msg_packet_in *in = (struct ofl_msg_packet_in *)**ome.msg;
    Port sport;

    if (!parse_lldp(in->data, in->data_length, sport)) {
        return CONTINUE;
    }

    Port dport(ome.dpid, /*in->in_port*/1);

    if (sport == dport) {
        lg.dbg("Loop detected.");
        return CONTINUE;
    }

    link_add(Link(sport, dport));

    return CONTINUE;
}

void
Discovery::dp_add(struct ofl_msg_features_reply *features) {
    datapathid dpid = datapathid::from_host(features->datapath_id);

    if (dps.find(dpid) != dps.end()) {
        lg.warn("Joined DP was already registered!");
        dp_remove(dpid);
    }

    lg.info("Adding DP %"PRIx64"", dpid.as_host());
    dps.insert(dpid);

    /* Ports don't come on features reply anymore*/
    /*for (size_t i = 0; i < features->ports_num; i++) {
        // sane ports only
        if (features->ports[i]->port_no > OFPP_MAX) {
            continue;
        }
        Port p(dpid, features->ports[i]->port_no);
        port_add(p);
    }*/

}

void
Discovery::dp_remove(const datapathid& dpid) {
    std::vector<Port>::iterator iter = ports.begin();

    while(iter != ports.end()) {
        if (iter->dpid == dpid) {
            iter = port_delete(iter++, Link_event::DP);
        } else {
            ++iter;
        }
    }

    lg.info("Removing DP %"PRIx64"", dpid.as_host());

    dps.erase(dpid);
}

void
Discovery::port_add(const Port& port) {
    std::vector<Port>::iterator iter = ports.begin();
    while (iter != ports.end()) {
        if (*iter == port) {
            lg.warn("Port was already registered!");
            return;
        }
        iter++;
    }

    lg.info("Adding Port %"PRIx64"-%"PRIx32"", port.dpid.as_host(), port.port);
    ports.push_back(port);
    lldps.insert(std::pair<Port, boost::shared_ptr<Buffer> >(port, boost::shared_ptr<Buffer>(create_lldp(port))));

    update_timeval();
}

void
Discovery::port_delete(const Port &port, Link_event::Reason reason) {
    std::vector<Port>::iterator iter = ports.begin();

    while (iter != ports.end()) {
        if (*iter == port) {
            port_delete(iter, reason);
            return;
        }
        iter++;
    }
}

std::vector<Port>::iterator
Discovery::port_delete(std::vector<Port>::iterator iter, Link_event::Reason reason) {
    std::map<Link, uint64_t>::iterator liter = links.begin();

    while(liter != links.end()) {
        if (liter->first.sport == *iter || liter->first.dport == *iter) {
                link_delete(liter++, reason);
        } else {
            ++liter;
        }
    }

    lg.info("Deleting Port %"PRIx64"-%"PRIx32"", iter->dpid.as_host(), iter->port);
    lldps.erase(*iter);

    std::vector<Port>::iterator ret = ports.erase(iter);

    update_timeval();

    return ret;
}

void
Discovery::link_add(const Link& link) {
    std::map<Link, uint64_t>::iterator iter = links.find(link);
    if (iter != links.end()) {
        iter->second = time_msec();
        return;
    }

    lg.info("Adding Link %"PRIx64"-%"PRIx32" -> %"PRIx64"-%"PRIx32"",
           link.sport.dpid.as_host(), link.sport.port,
           link.dport.dpid.as_host(), link.dport.port);
    links[link] = time_msec();

    Link_event *event = new Link_event(link.sport.dpid, link.dport.dpid,
                                        link.sport.port, link.dport.port,
                                        Link_event::ADD, Link_event::LINK);

    nox::post_event(event);

}

void
Discovery::link_delete(const Link& link, Link_event::Reason reason) {
    std::map<Link, uint64_t>::iterator iter = links.begin();

    while(iter != links.end()) {
        if (iter->first == link) {
            link_delete(iter, reason);
            return;
        }
        iter++;
    }
}

void
Discovery::link_delete(std::map<Link, uint64_t>::iterator iter, Link_event::Reason reason) {
    Link_event *event = new Link_event(iter->first.sport.dpid, iter->first.dport.dpid,
                                       iter->first.sport.port, iter->first.dport.port,
                                        Link_event::REMOVE, reason);

    lg.info("Deleting Link %"PRIx64"-%"PRIx32" -> %"PRIx64"-%"PRIx32"",
            iter->first.sport.dpid.as_host(), iter->first.sport.port,
            iter->first.dport.dpid.as_host(), iter->first.dport.port);

    links.erase(iter);

    nox::post_event(event);
}


void
Discovery::update_timeval() {
    uint64_t pp = ports.size() == 0 ? per_port
                                    : std::max(packet_out, per_port / ports.size());
    per_port_tv.tv_sec  =  pp / 1000;
    per_port_tv.tv_usec = (pp % 1000) * 1000;

    lg.dbg("Per_port set to: %"PRIu64"ms. ", pp);
}

void
Discovery::timeout_links() {
    post(boost::bind(&Discovery::timeout_links, this), timeout_tv);

    uint64_t last_time = time_msec() - link_timeout;

    std::map<Link, uint64_t>::iterator iter = links.begin();
    while(iter != links.end()) {
        if (iter->second < last_time) {
            lg.warn("Link Timed out %"PRIx64"-%"PRIx32" -> %"PRIx64"-%"PRIx32"",
                    iter->first.sport.dpid.as_host(), iter->first.sport.port,
                    iter->first.dport.dpid.as_host(), iter->first.dport.port);
            link_delete(iter++, Link_event::LINK);
        } else {
            ++iter;
        }
    }
}

void
Discovery::send_lldp() {
    post(boost::bind(&Discovery::send_lldp, this), per_port_tv);

    if (ports.empty()) { return; }

    if (ports.size() <= next_port) {
        next_port = 0;
    }

    Port& port = ports.at(next_port);

    std::map<Port, boost::shared_ptr<Buffer> >::iterator iter = lldps.find(port);
    if (iter == lldps.end()) {
        lg.warn("Missing LLDP packet");
        return;
    }

    send_openflow_pkt(port.dpid, *(iter->second), OFPP_CONTROLLER, port.port, false);
    next_port++;
}


Buffer *
Discovery::create_lldp(const Port& port, uint16_t ttl) {
    uint8_t *lldp = new uint8_t[36]; // eth=14,tlv1=9,tlv2=7,tlv3=4,tlv0=2
    uint64_t dpid = port.dpid.as_net();

    struct eth_header *eth = (struct eth_header *)lldp;
    memcpy(eth->eth_src, ((uint8_t *)&dpid)+2, 6); memset(eth->eth_src, 0x00, 1);
    memcpy(eth->eth_dst, "\01\23\20\00\00\01", 6);
    eth->eth_type = htons(LLDP_TYPE);

    uint8_t *chassis_tlv = lldp + 14;
    memcpy(chassis_tlv,   "\02\07", 2);                    // type=1(chassis), len=7
    memcpy(chassis_tlv+2, "\04", 1);                       // subtype=4(mac)
    memcpy(chassis_tlv+3, ((uint8_t *)&dpid)+2, 6); // id = dpid[2:7]

    uint8_t *port_tlv = chassis_tlv + 9;
    memcpy(port_tlv,   "\04\05", 2);      // type=2(port), len=5
    memcpy(port_tlv+2, "\02", 1);         // subtype=2(port)
    memcpy(port_tlv+3, &port.port, 4);    // id = port_id

    uint8_t *ttl_tlv = port_tlv + 7;
    memcpy(ttl_tlv,   "\06\02", 2);  // type=3(ttl), len=2
    memcpy(ttl_tlv+2, &ttl, 2);      // ttl = ttl

    uint8_t *end_tlv = ttl_tlv + 4;
    memset(end_tlv, 0x00, 2);      // type=0(end), len=0

    return new Array_buffer(lldp, 36);
}

// if packet is lldp, returns true and fills port
bool
Discovery::parse_lldp(const uint8_t *buf, size_t buf_len, Port& port) {
    if (buf_len < 36) { lg.dbg("Packet is too short for LLDP"); return false; }

    struct eth_header *eth = (struct eth_header *)buf;
    if (eth->eth_type != htons(LLDP_TYPE)) { lg.dbg("Packet is not LLDP type"); return false; }

    uint8_t *chassis_tlv = (uint8_t *)eth + 14;
    if (memcmp(chassis_tlv,   "\02\07", 2) != 0) { lg.dbg("Expected chassis TLV of len 7"); return false; }
    if (memcmp(chassis_tlv+2, "\04", 1)    != 0) { lg.dbg("Chassis TLV is not MAC subtype"); return false; }
    port.dpid = datapathid::from_bytes(chassis_tlv+3);

    uint8_t *port_tlv = chassis_tlv + 9;
    if (memcmp(port_tlv,   "\04\05", 2) != 0) { lg.dbg("Expected port TLV of len 5"); return false; }
    if (memcmp(port_tlv+2, "\02", 1)    != 0) { lg.dbg("Port TLV is not Port subtype"); return false; }
    port.port = *((uint32_t *)(port_tlv+3));

    return true;
}

REGISTER_COMPONENT(container::Simple_component_factory<Discovery>, Discovery);

} // unnamed namespace
