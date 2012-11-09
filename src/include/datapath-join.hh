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
#ifndef DATAPATH_JOIN_HH
#define DATAPATH_JOIN_HH 1

#include "port.hh"
#include "event.hh"
#include "netinet++/datapathid.hh"
#include "ofp-msg-event.hh"
#include "ofp-msg.hh"

#include "openflow/openflow.h"

namespace vigil {

/** \ingroup noxevents
 *
 * Datapath_join_events are thrown for each new switch which connects to
 * NOX.
 *
 */

struct Datapath_join_event
    : public Ofp_msg_event
{
    Datapath_join_event(datapathid dpid, uint32_t xid, boost::shared_ptr<Ofp_msg> msg) :
        Ofp_msg_event(static_get_name(), dpid, xid, msg) {
            assert((*msg)->type == OFPT_FEATURES_REPLY);
        };

    // -- only for use within python
    //Datapath_join_event() : Event(static_get_name()) { }

    static const Event_name static_get_name() {
        return "Datapath_join_event";
    }

/*
    Datapath_join_event(const Datapath_join_event&);
    Datapath_join_event& operator=(const Datapath_join_event&);
*/
};

/*
inline
Datapath_join_event::Datapath_join_event(const Datapath_join_event& dje)
  : Event(static_get_name()), Ofp_msg_event(dje.get_ofp_msg(), dje.buf),
    n_buffers(dje.n_buffers), n_tables(dje.n_tables),
    capabilities(dje.capabilities), mgmt_id(dje.mgmt_id)
{
  datapath_id = dje.datapath_id;
  std::vector<Port>::const_iterator i = dje.ports.begin();
  while (i != dje.ports.end())
  {
    ports.push_back(Port(*i));
    i++;
  }
}

inline
Datapath_join_event::Datapath_join_event(const ofp_switch_features *osf,
                                         std::auto_ptr<Buffer> buf,
                                         datapathid mgmt_id_)
    : Event(static_get_name()), Ofp_msg_event(&osf->header, buf)
{
    datapath_id  = datapathid::from_net(osf->datapath_id); 
    n_buffers    = ntohl(osf->n_buffers);
    n_tables     = osf->n_tables;
    capabilities = ntohl(osf->capabilities);
    mgmt_id      = mgmt_id_;

    / * The rest of this message is an array of ports * /
    int port_count = (ntohs(osf->header.length) - sizeof(*osf)) 
                / sizeof(*osf->ports);

    for (int i=0; i<port_count; ++i) {
        ports.push_back(&osf->ports[i]);
    }
}
*/

} // namespace vigil

#endif /* datapath-join.hh */
