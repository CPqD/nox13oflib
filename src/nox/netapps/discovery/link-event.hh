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
#ifndef LINK_EVENT_HH
#define LINK_EVENT_HH 1

#include <boost/noncopyable.hpp>
#include <stdint.h>

#include "event.hh"
#include "netinet++/datapathid.hh"

namespace vigil {

/** \ingroup noxevents
 *
 * Thrown for each link status change detected on the network.
 * Currently is only thrown by the discovery component. 
 *
 */


struct Link_event
    : public Event,
      boost::noncopyable
{
    enum Action {
        ADD,
        REMOVE
    };

    enum Reason {
    	LINK,  /* in case of REMOVE it means a link failure; always used for add */
    	PORT,  /* in case of REMOVE it means a link is down because of a port */
    	DP     /* in case of REMOVE it means the link is removed because of a dp leave */
    };

    Link_event(datapathid dpsrc_, datapathid dpdst_,
               uint32_t sport_, uint32_t dport_,
               Action action_, Reason reason_);

    static const Event_name static_get_name() {
        return "Link_event";
    }

    datapathid dpsrc;
    datapathid dpdst;
    uint32_t   sport;
    uint32_t   dport;
    Action     action;
    Reason     reason;
};

} // namespace vigil

#endif /* link-event.hh */
