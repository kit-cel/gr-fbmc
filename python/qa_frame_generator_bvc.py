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


class qa_frame_generator_bvc(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(2, linewidth=150)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = self._get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = 8
        overlap = 4

        preamble = self._get_preamble(total_subcarriers)
        payload = self._get_payload(payload_symbols, used_subcarriers)
        frame = self._get_frame(payload, total_subcarriers, channel_map, payload_symbols, overlap)
        print np.reshape(frame, (-1, total_subcarriers)).T

        # create and connect blocks
        src = blocks.vector_source_b(payload, repeat=False)
        framer = fbmc.frame_generator_bvc(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble)
        snk = blocks.vector_sink_c(total_subcarriers)
        self.tb.connect(src, framer, snk)

        # run fg
        self.tb.run()
        # check data

        res = snk.data()
        print np.reshape(res, (-1, total_subcarriers)).T

    def _get_payload(self, payload_symbols, used_subcarriers):
        num_pl_vals = payload_symbols * used_subcarriers // 2
        payload = np.random.randint(0, 2, num_pl_vals)
        return payload

    def _get_channel_map(self, used_subcarriers, total_subcarriers):
        # channel_map = [1, 1, 0, 0, 1, 1, 0, 0]  # original!
        channel_map = np.concatenate((np.zeros(total_subcarriers - used_subcarriers, dtype=int), np.ones(used_subcarriers, dtype=int)))
        np.random.shuffle(channel_map)  # careful! it's an in-place shuffle!
        return channel_map

    def _get_preamble(self, total_subcarriers):
        preamble = np.ones(total_subcarriers, dtype=float) * 2
        preamble = np.concatenate((preamble, np.zeros(total_subcarriers, dtype=float)))
        preamble = np.concatenate((preamble, np.ones(total_subcarriers, dtype=float) * 4))
        preamble = np.concatenate((preamble, np.ones(total_subcarriers, dtype=float)))
        return preamble

    def _get_padding_zeros(self, total_subcarriers, overlap):
        return np.zeros(total_subcarriers * overlap, dtype=float)

    def _get_payload_frame(self, payload, total_subcarriers, channel_map, payload_symbols):
        if payload_symbols % 2 != 0:
            raise ValueError('payload_symbols MUST be even!')
        invsqrt = 1.0 / np.sqrt(2.0)
        pf = np.zeros((payload_symbols, total_subcarriers), dtype=float)
        smt_scheme = self._get_smt_scheme(total_subcarriers)
        ctr = 0
        for i, vec in enumerate(pf):
            for e, el in enumerate(vec):
                if channel_map[e] != 0 and smt_scheme[i % 2][e] != 0:
                    val = payload[ctr]
                    pf[i][e] = (2 * val - 1) * invsqrt
                    ctr += 1
        return pf.flatten()

    def _get_smt_scheme(self, total_subcarriers):
        scheme = np.zeros((2, total_subcarriers), dtype=int)
        isInphase = True
        for i in range(total_subcarriers):
            if isInphase:
                scheme[0][i] = 1
            if not isInphase:
                scheme[1][i] = 1
            if i % 2 == 0:
                isInphase = not isInphase
        return scheme

    def _get_frame(self, payload, total_subcarriers, channel_map, payload_symbols, overlap):
        preamble = self._get_preamble(total_subcarriers)
        zero_pad = self._get_padding_zeros(total_subcarriers, overlap)
        payload = self._get_payload_frame(payload, total_subcarriers, channel_map, payload_symbols)
        return np.concatenate((preamble, zero_pad, payload, zero_pad))

if __name__ == '__main__':
    gr_unittest.run(qa_frame_generator_bvc)
