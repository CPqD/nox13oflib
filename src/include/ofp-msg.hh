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
#ifndef OFP_MSG_HH
#define OFP_MSG_HH

#include "../oflib/ofl-messages.h"

namespace vigil
{

// wrapper for ofl_msg structures
class Ofp_msg {
public:
    Ofp_msg(struct ::ofl_msg_header *msg_) : msg(msg_) { };

    struct ofl_msg_header * operator*() const {
        return msg;
    };

    struct ofl_msg_header * operator->() const {
        return msg;
    };


    ~Ofp_msg(void) {
        ofl_msg_free(msg, NULL/*ofl_exp*/);
    };

private:
    struct ::ofl_msg_header *msg;
};

} // namespace vigil

#endif  // -- OFP_MSG_HH
