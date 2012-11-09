#ifndef DISCOVERY_HH
#define DISCOVERY_HH

#include <boost/shared_array.hpp>
#include <map>
#include <vector>
#include <set>
#include "component.hh"
#include "timeval.hh"
#include "link-event.hh"

#include "../../../oflib/ofl-messages.h"


#define LLDP_TTL               120

/*NOTE: Discovery will try to keep the per-port interval
        unless it would result in a less than packet out
        interval.
        Values are in ms.
*/

#define PER_PORT_PERIOD        100
#define PACKET_OUT_PERIOD       10
#define TIMEOUT_CHECK_PERIOD   500
#define LINK_TIMEOUT           PER_PORT_PERIOD * 3

#define LLDP_TYPE           0x88cc


namespace {

using namespace vigil;
using namespace vigil::container;

class Port {
public:
    Port() : dpid(datapathid()), port(0) { };
    Port(const datapathid& _dpid, uint32_t _port) :
        dpid(_dpid), port(_port) { };

    bool operator==(const Port& p) const {
      return (dpid == p.dpid) &&
             (port == p.port);
    }

    bool operator!=(const Port& p) const {
        return (dpid != p.dpid) ||
               (port != p.port);
    }

    bool operator<(const Port& p) const {
        if (dpid == p.dpid) {
            return (port < p.port);
        }
        return (dpid < p.dpid);
    }

    datapathid dpid;
    uint32_t port;
};



class Link {
public:
    Link(const Port& _sport, const Port& _dport) :
        sport(_sport), dport(_dport) { };

    bool operator==(const Link& l) const {
      return (sport == l.sport) &&
             (dport == l.dport);
    }

    bool operator!=(const Link& l) const {
        return (sport != l.sport) ||
               (dport != l.dport);
    }

    bool operator<(const Link& l) const {
        if (sport == l.sport) {
            return (dport < l.dport);
        }
        return (sport < l.sport);
    }

    Port sport;
    Port dport;
};

class Discovery : public Component {
public:
     Discovery(const Context* c, const json_object*);
    void configure(const Configuration *c);
    void install();

    Disposition dp_join_handler(const Event& e);
    Disposition dp_leave_handler(const Event& e);
    Disposition port_status_handler(const Event& e);
    Disposition packet_in_handler(const Event& e);

private:
    void dp_add(struct ofl_msg_features_reply *features);
    void dp_remove(const datapathid& dpid);
    void port_add(const Port& port);
    void port_delete(const Port &port, Link_event::Reason reason);
    std::vector<Port>::iterator port_delete(std::vector<Port>::iterator iter, Link_event::Reason reason);
    void link_add(const Link& link);
    void link_delete(const Link& link, Link_event::Reason reason);
    void link_delete(std::map<Link, uint64_t>::iterator iter, Link_event::Reason reason);

    void update_timeval();
    void timeout_links();
    void send_lldp();

    std::set<datapathid> dps;
    std::vector<Port> ports;
    std::map<Port, boost::shared_ptr<Buffer> > lldps;
    std::map<Link, uint64_t> links;

    size_t next_port;

    uint64_t per_port;
    uint64_t packet_out;
    uint64_t timeout;
    uint64_t link_timeout;
    timeval per_port_tv;
    timeval timeout_tv;

    static Buffer *create_lldp(const Port& port, uint16_t ttl = LLDP_TTL);
    static bool parse_lldp(const uint8_t *buf, size_t buf_len, Port& port);
};


} // unnamed namespace

#endif /* DISCOVERY_HH */
