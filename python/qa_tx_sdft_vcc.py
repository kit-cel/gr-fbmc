#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014 <+YOU OR YOUR COMPANY+>.
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


class qa_tx_sdft_vcc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        multiple = 5
        overlap = 1  # must be 1 in this test!
        L = 4
        taps = np.array([0, 1, 2, 1]) #  np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L + 1, dtype=np.complex)

        tx_sdft = fbmc.tx_sdft_vcc(taps, L)
        src = blocks.vector_source_c(data, vlen=L)
        snk = blocks.vector_sink_c(vlen=1)
        # set up fg
        self.tb.run()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_tx_sdft_vcc, "qa_tx_sdft_vcc.xml")
