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

#ifndef PORT_STATS_HH__
#define PORT_STATS_HH__

#include <string>

#include "event.hh"
#include "netinet++/datapathid.hh"
#include "ofp-msg-event.hh"

#include "openflow/openflow.h"

#include "../oflib/ofl-structs.h"

namespace vigil {



struct Port_stats
{
    Port_stats(uint32_t pn) :
        port_no(pn), 
        rx_packets(0), tx_packets(0), rx_bytes(0), tx_bytes(0),
        rx_dropped(0), tx_dropped(0), rx_errors(0), tx_errors(0),
        rx_frame_err(0), rx_over_err(0), rx_crc_err(0), collisions(0) { ; }

    Port_stats(const Port_stats& ps) :
        port_no(ps.port_no),
	rx_packets(ps.rx_packets), 
        tx_packets(ps.tx_packets), 
        rx_bytes(ps.rx_bytes), 
        tx_bytes(ps.tx_bytes), 
        rx_dropped(ps.rx_dropped), 
        tx_dropped(ps.tx_dropped), 
        rx_errors(ps.rx_errors), 
        tx_errors(ps.tx_errors), 
        rx_frame_err(ps.rx_frame_err), 
        rx_over_err(ps.rx_over_err), 
        rx_crc_err(ps.rx_crc_err), 
        collisions(ps.collisions) { ; }

    Port_stats(struct ofl_port_stats *ops) :
        port_no(ops->port_no),
        rx_packets(ops->rx_packets),
        tx_packets(ops->tx_packets),
        rx_bytes(ops->rx_bytes),
        tx_bytes(ops->tx_bytes),
        rx_dropped(ops->rx_dropped),
        tx_dropped(ops->tx_dropped),
        rx_errors(ops->rx_errors),
        tx_errors(ops->tx_errors),
        rx_frame_err(ops->rx_frame_err),
        rx_over_err(ops->rx_over_err),
        rx_crc_err(ops->rx_crc_err),
        collisions(ops->collisions) { ; }

    uint16_t port_no;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_dropped;
    uint64_t tx_dropped;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t rx_frame_err;
    uint64_t rx_over_err;
    uint64_t rx_crc_err;
    uint64_t collisions;

    Port_stats& operator=(const Port_stats& ps)
    {
    port_no = ps.port_no;
	rx_packets = ps.rx_packets;
	tx_packets = ps.tx_packets;
	rx_bytes = ps.rx_bytes;
	tx_bytes = ps.tx_bytes;
	rx_dropped = ps.rx_dropped;
	tx_dropped = ps.tx_dropped;
	rx_errors = ps.rx_errors;
	tx_errors = ps.tx_errors;
	rx_frame_err = ps.rx_frame_err;
	rx_over_err = ps.rx_over_err;
	rx_crc_err = ps.rx_crc_err;
	collisions = ps.collisions;

	return *this;
    }
};

} // namespace vigil

#endif // PORT_STATS_HH__
