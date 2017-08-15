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

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import fbmc_swig as fbmc
import fbmc_test_functions as ft


class qa_serial_to_parallel_cvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        inlen = 3
        outlen = 5
        channel_map = (0, 1, 1, 0, 1)
        d = range(1, inlen + 1)
        ref = ft.map_to_channel(d, inlen, outlen, channel_map)
        multiple = 100
        d *= multiple
        ref *= multiple

        self.src = blocks.vector_source_c(d)
        self.s2p = fbmc.serial_to_parallel_cvc(inlen, outlen, channel_map)
        self.snk = blocks.vector_sink_c(vlen=outlen)
        self.tb.connect(self.src, self.s2p, self.snk)
        self.tb.run()
        # check data

        data = self.snk.data()
        self.assertComplexTuplesAlmostEqual(ref, data)

    def test_002_t(self):
        n = 100000
        input_data = range(n)
        self.src = blocks.vector_source_c(input_data)
        self.s2p = fbmc.serial_to_parallel_cvc(10, 16, (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0))
        self.snk = blocks.vector_sink_c(vlen=16)
        self.tb.connect(self.src, self.s2p, self.snk)
        self.tb.run()
        # check data
        data = self.snk.data()
        self.assertEqual(n * 16 / 10, len(data))


if __name__ == '__main__':
    gr_unittest.run(qa_serial_to_parallel_cvc)
