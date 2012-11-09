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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef OFP_MSG_EVENT_HH
#define OFP_MSG_EVENT_HH

#include <boost/shared_ptr.hpp>
#include "buffer.hh"
#include "netinet++/datapathid.hh"
#include "event.hh"
#include "ofp-msg.hh"

#include <iostream>

namespace vigil
{

class Ofp_msg_event : public Event {
public:
    const datapathid dpid;
    const uint32_t xid;
    const boost::shared_ptr<Ofp_msg> msg;

    static Ofp_msg_event *create_event(datapathid dpid, uint32_t xid, boost::shared_ptr<Ofp_msg> msg);

    static std::string get_name(enum ofp_type type);
    static std::string get_stats_name(enum ofp_multipart_types type);
protected:
    Ofp_msg_event(std::string name, datapathid _dpid, uint32_t _xid, boost::shared_ptr<Ofp_msg> _msg) :
        Event(name), dpid(_dpid), xid(_xid), msg(_msg) { };
};

} // namespace vigil

#endif  // -- OFP_MSG_EVENT_HH
