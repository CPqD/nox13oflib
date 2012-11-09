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
#include "flow.hh"


#include <ostream>
#include <stdexcept>

#include "netinet++/ethernet.hh"
#include "netinet++/ip.hh"

#include <netinet/in.h>

#include "vlog.hh"
#include "buffer.hh"
#include "openflow/openflow.h"
#include "packets.h"
#include "vlog.hh"
#include "openssl/md5.h"

#include <inttypes.h>
#include <netinet/in.h>
#include <string.h>
#include <config.h>
#include <sys/types.h>


namespace vigil {

static Vlog_module log("flow");

Flow::Flow() {
	init();
}
/** Constructor from ofp_match
 */
Flow::Flow(const struct ofl_match *match_) {
	init();
	memcpy(&match, match_, sizeof(struct ofl_match));
}

void
Flow::init() {
    ofl_structs_match_init(&this->match);
}

const std::string
Flow::to_string() const
{
	char *s = ofl_structs_match_to_string((struct ofl_match_header *)&match, NULL);
	const std::string ret(s);
	free(s);

	return ret;
}

std::ostream&
operator<<(std::ostream& stream, const Flow& f) 
{
    return stream << f.to_string();
}

uint64_t
Flow::hash_code() const
{
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, &match, sizeof(struct ofl_match));
    MD5_Final(md, &ctx);

    return *((uint64_t*)md);
}

bool operator==(const Flow& lhs, const Flow& rhs)
{
  return memcmp(&lhs.match, &rhs.match, sizeof(struct ofl_match)) == 0;
}

bool operator!=(const Flow& lhs, const Flow& rhs)
{
  return !(lhs == rhs);
}

} // namespace vigil

