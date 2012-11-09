/** Copyright 2009 (C) Stanford University
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
#ifndef OPENFLOW_PACK_H
#define OPENFLOW_PACK_H

//#include "openflow-pack-raw.hh"
#include "buffer.hh"
#include "../libopenflow/byte-order.h"
#include <boost/shared_array.hpp>
#include <stdlib.h>
#include <string.h>

/** \brief Minimum transaction id.
 *
 * All xid smaller than this value are reserved 
 * and will not be used.
 */
#define OPENFLOW_PACK_MIN_XID 256

/** \brief Mode of generating transaction id.
 * 
 * Defaults to increment if not defined.
 */
#define OPENFLOW_PACK_XID_METHOD_INCREMENT 0
#define OPENFLOW_PACK_XID_METHOD_RANDOM 1
#define OPENFLOW_PACK_XID_METHOD_PRIVSPACE 2
#define OPENFLOW_PACK_XID_METHOD OPENFLOW_PACK_XID_METHOD_PRIVSPACE

namespace vigil
{
  /** \brief Raw classes to help pack OpenFlow messages
   * \ingroup noxapi
   *
   * @author ykk
   * @date May 2010
   * @file openflow-pack-raw.hh
   * @file openflow.h
   */

  /** \brief Class to help pack OpenFlow messages
   * \ingroup noxapi
   *
   * @see <A HREF="openflow-pack-raw_8hh.html">openflow-pack-raw</A>
   * @author ykk
   * @date May 2010
   */
  class openflow_pack
  {
  public:
    /** \brief Get next xid for OpenFlow
     *
     * @return xid to use
     */
    static uint32_t get_xid();
  private:
    /** Next OpenFlow xid (if method is increment)
     */
    static uint32_t nextxid;

    /** \brief Random unsigned 32 bit integer
     *
     * @return random unsigned long
     */
    static uint32_t rand_uint32();
  };
} // namespace vigil

#endif
