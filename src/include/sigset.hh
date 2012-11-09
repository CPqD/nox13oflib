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
#ifndef SIGSET_HH
#define SIGSET_HH 1

#include <limits.h>
#include <signal.h>

namespace vigil {

/* A set of Unix signals; wrapper for sigset_t. */
class Sigset
{
public:
    Sigset();
    Sigset(const sigset_t&);
    void clear();
    void fill();
    int scan(int start = 1) const;
    bool contains(int sig_nr) const;
    void add(int sig_nr);
    void remove(int sig_nr);
    const sigset_t& sigset() const;
    const Sigset& operator|=(const Sigset&);
    const Sigset& operator&=(const Sigset&);
private:
    sigset_t ss;

    static int n_sig();
};

Sigset operator|(const Sigset&, const Sigset&);
Sigset operator&(const Sigset&, const Sigset&);

} // namespace vigil

#endif /* sigset.hh */
