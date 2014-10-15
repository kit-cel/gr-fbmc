#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from operator import add


def generate_vector(L):
    d = [complex(i, i + L) for i in range(0, L)]
    return d


def serialize_vector(d):
    L = int(len(d))
    di = [complex(d[i].real, 0) for i in range(L)]
    dq = [complex(d[i].imag, 0) for i in range(L)]
    di.extend(dq)
    return di


def map_to_channel(d, inlen, outlen, channel_map):
    assert(len(d) == inlen)
    assert(len(channel_map) == outlen)
    v = range(outlen)
    num = 0
    for i in range(outlen):
        if channel_map[i] == 1:
            v[i] = d[num]
            num += 1
        else:
            v[i] = complex(0, 0)
    return v


def unmap_from_channel(d, inlen, outlen, channel_map):
    assert(len(d) == inlen)
    assert(len(channel_map) == inlen)
    assert(len([i for i in channel_map if i == 1]) == outlen)

    num = 0
    v = range(outlen)
    for i in range(inlen):
        if channel_map[i] == 1:
            v[num] = d[i]
            num += 1
    return v


def commutate_output(d, L):
    assert(len(d) == L)
    assert(L % 2 == 0)
    Ld2 = int(L / 2)
    return map(add, d[0:Ld2], d[Ld2:])


def commutate_input(d, buf, L):
    # careful not exactly the inverse to commutate output!
    assert(len(buf) >= L/2 - 1)  # make sure enough history is provided!
    assert(len(d) > L - 2)  # require one symbol!
    offset = L/2 - 1
    buf = buf[0:offset] + d
    res = []
    for k in range(2):
        revbuf = buf[0:L/2]
        revbuf = revbuf[::-1]
        revbuf *= 2
        res += revbuf
        buf = buf[L/2:]
    return [res, buf]


def commutate_input_stream(d, L):
    assert(len(d) % L == 0)  # otherwise we may produce unexpected results.
    buf = [0, ] * 2 * L
    res = []
    while len(d) >= L - 1:
        [r, buf] = commutate_input(d[0:L], buf, L)
        res = res + r
        d = d[L:]
    return res


def main():
    print "fbmc_test_functions"
    L = 6

    d = range(1, 4 * L + 1)
    print len(d)
    res = commutate_input_stream(d, L)
    print len(res)
    print res
    # buf = [0, ] * 2 * L
    # commutate_input(d[0:L], buf, L)

if __name__ == '__main__':
    main()