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
#include "packet-classifier.hh"
#include <boost/bind.hpp>
#include "assert.hh"
#include "nox.hh"
#include "flow.hh"
//#include "packet-in.hh"
#include "ofp-msg-event.hh"
#include "../oflib/ofl-messages.h"

namespace vigil {

void
Packet_classifier::register_packet_in()
{
    nox::register_handler("Packet_in_event",
                            boost::bind(&Packet_classifier::handle_packet_in,
                                        this, _1), 100);
}

Disposition
Packet_classifier::handle_packet_in(const Event& e)
{
    const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
    struct ofl_msg_packet_in *opi = (struct ofl_msg_packet_in *)**pi.msg;

    const Rule<Packet_expr, Pexpr_action> *match;
    //Flow flow(opi->in_port, Nonowning_buffer(opi->data, opi->data_length));

    //result.set_data(&flow);
    get_rules(result);
    match = result.next();
    if (match == NULL) {
        result.clear();
        return CONTINUE;
    }
    int top_priority = match->priority;
    do {
        match->action(pi);
        match = result.next();
    } while (match != NULL && (match->priority == top_priority));

    result.clear();

    return CONTINUE;
}

}
