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
#ifndef SOCKET_UTIL_HH
#define SOCKET_UTIL_HH 1

#include <sys/socket.h>
#include <cerrno>
#include <fcntl.h>
#include "errno_exception.hh"
#include "netinet++/ipaddr.hh"

namespace vigil {

static inline sockaddr_in
make_sin(ipaddr ip, uint16_t port)
{
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    sin.sin_port = port;
    return sin;
}

static inline void
set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw errno_exception(errno, "fcntl");
    }
}

} // namespace vigil

#endif /* socket-util.hh */
