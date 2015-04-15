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
import numpy as np
import fbmc_test_functions as ft


class qa_tx(gr_unittest.TestCase):
    def get_frame_len(self):
        return self.cfg.num_total_subcarriers() * self.cfg.num_sym_frame()

    def setUp(self):
        np.set_printoptions(2, linewidth=150)
        self.tb = gr.top_block()
        self.cfg = fbmc.fbmc_config(num_used_subcarriers=20, num_payload_sym=16, num_overlap_sym=4, modulation="QPSK",
                                    preamble="IAM", samp_rate=250000)

    def tearDown(self):
        self.tb = None

    def test_001_frames(self):
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

    def test_002_modulation(self):
        total_subcarriers = 8
        overlap = 4
        taps = ft.generate_phydyas_filter(total_subcarriers, overlap)

        data = np.arange(1, 2 * total_subcarriers * overlap + 1, dtype=complex)

        src = blocks.vector_source_c(data, vlen=total_subcarriers, repeat=False)
        mod = fbmc.tx_sdft_vcc(taps, total_subcarriers)
        demod = fbmc.rx_sdft_cvc(taps, total_subcarriers)
        snk = blocks.vector_sink_c(total_subcarriers)

        self.tb.connect(src, mod, demod, snk)
        self.tb.run()

        res = np.array(snk.data())
        print np.reshape(res, (-1, total_subcarriers)).T

        # self.assertTupleEqual(tuple(payload), tuple(res))


if __name__ == '__main__':
    gr_unittest.run(qa_tx)
