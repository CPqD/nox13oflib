/* Copyright 2008 (C) Nicira, Inc.
 *
 * This file is part of NOX.
 *
 * NOX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NOX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NOX.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <netinet/in.h>
#include "assert.hh"
#include "component.hh"
#include "flow.hh"
//#include "packet-in.hh"
#include "ofp-msg-event.hh"
#include "vlog.hh"
#include "flowmod.hh"

#include "netinet++/ethernet.hh"

#include "../../../oflib/ofl-structs.h"
#include "../../../oflib/ofl-messages.h"

namespace {

using namespace vigil;
using namespace vigil::container;

Vlog_module lg("hub");

class Hub 
    : public Component 
{
public:
     Hub(const Context* c,
         const json_object*) 
         : Component(c) { }
    
    void configure(const Configuration*) {
    }

    Disposition handler(const Event& e)
    {
        const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
        struct ofl_msg_packet_in *in = (struct ofl_msg_packet_in *)**pi.msg;

       Flow *flow = new Flow((struct ofl_match*) in->match);

        /* drop all LLDP packets */
        uint16_t dl_type;
        flow->get_Field("eth_type",&dl_type);
        if (dl_type == ethernet::LLDP){
            return CONTINUE;
        }
        Flow fl;
        Actions *act = new Actions();
        act->CreateOutput(OFPP_FLOOD);
        Instruction *inst = new Instruction();
        inst->CreateApply(act);
        FlowMod *mod = new FlowMod(0x00ULL,0x00ULL,0,OFPFC_ADD, 5, 5, OFP_DEFAULT_PRIORITY, 
                                  in->buffer_id, OFPP_ANY, OFPG_ANY,0x00000);
        mod->AddInstructions(inst);
        mod->AddMatch(&fl.match);
        send_openflow_msg(pi.dpid, (struct ofl_msg_header *)&mod->fm_msg, 0/*xid*/, true/*block*/);

        if (in->buffer_id == UINT32_MAX) {
            if (in->total_len == in->data_length) {
                uint32_t in_port;
                flow->get_Field("in_port", &in_port);
                send_openflow_pkt(pi.dpid, Array_buffer(in->data, in->data_length), in_port, OFPP_FLOOD, true/*block*/);
            } else {
                /* Control path didn't buffer the packet and didn't send us
                 * the whole thing--what gives? */
                lg.dbg("total_len=%zu data_len=%zu\n", in->total_len, in->data_length);
            }
        }

        return CONTINUE;
    }

    void install()
    {
        register_handler(Ofp_msg_event::get_name(OFPT_PACKET_IN), boost::bind(&Hub::handler, this, _1));
    }
};

REGISTER_COMPONENT(container::Simple_component_factory<Hub>, Hub);

} // unnamed namespace

