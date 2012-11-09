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

#ifndef TABLE_STATS_HH__
#define TABLE_STATS_HH__

#include <string>

#include "event.hh"
#include "netinet++/datapathid.hh"
#include "ofp-msg-event.hh"

#include "openflow/openflow.h"

#include "../oflib/ofl-structs.h"

namespace vigil {

//TODO: missing 1.1 fields
struct Table_stats
{
    Table_stats(struct ofl_table_stats *stats) :
        table_id(stats->table_id), name(stats->name),
        max_entries(stats->max_entries), active_count(stats->active_count),
        matched_count(stats->matched_count) { }
    Table_stats(int tid, const std::string& n, uint32_t me,
            uint32_t ac, uint64_t lc, uint64_t mc):
        table_id(tid), name(n),
        max_entries(me), active_count(ac),
        lookup_count(lc), matched_count(mc){ ; }

    int table_id;
    std::string name;
    uint32_t max_entries;
    uint32_t active_count;
    uint64_t lookup_count;
    uint64_t matched_count;
};


} // namespace vigil

#endif // TABLE_STATS_HH__
