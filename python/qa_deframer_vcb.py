#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2015 <+YOU OR YOUR COMPANY+>.
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
import numpy as np
import fbmc_test_functions as ft


class qa_deframer_vcb(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(2, linewidth=150)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = ft.get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = 8
        overlap = 4

        payload = ft.get_payload(payload_symbols, used_subcarriers)
        frame = ft.get_frame(payload, total_subcarriers, channel_map, payload_symbols, overlap)
        frame = np.concatenate((frame, frame))
        payload = np.concatenate((payload, payload))

        # set up fg
        src = blocks.vector_source_c(frame, repeat=False, vlen=total_subcarriers)
        deframer = fbmc.deframer_vcb(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map)
        snk = blocks.vector_sink_b(1)
        self.tb.connect(src, deframer, snk)
        self.tb.run()

        # check data
        res = np.array(snk.data())
        print res
        print payload

        self.assertTupleEqual(tuple(payload), tuple(res))

    def test_002_chain_test(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = ft.get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = 8
        overlap = 4

        preamble = ft.get_preamble(total_subcarriers)
        payload = ft.get_payload(payload_symbols, used_subcarriers)
        payload = np.concatenate((payload, payload))

        src = blocks.vector_source_b(payload, repeat=False)
        framer = fbmc.frame_generator_bvc(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble)
        deframer = fbmc.deframer_vcb(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map)
        snk = blocks.vector_sink_b(1)
        self.tb.connect(src, framer, deframer, snk)
        self.tb.run()

        res = np.array(snk.data())
        print res
        print payload

        self.assertTupleEqual(tuple(payload), tuple(res))

if __name__ == '__main__':
    gr_unittest.run(qa_deframer_vcb, "qa_deframer_vcb.xml")
