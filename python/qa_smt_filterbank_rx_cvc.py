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
import fbmc_test_functions as ft
import numpy as np


class qa_smt_filterbank_rx_cvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        multiple = 100
        L = 16
        overlap = 4
        taps = range(overlap * L + 1)
        d = range(L * multiple)

        # set up fg
        self.src = blocks.vector_source_c(d, vlen=1)
        self.smt = fbmc.smt_filterbank_rx_cvc(taps, L)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.smt, self.snk)
        self.tb.run()
        # check data

        res = self.snk.data()
        self.assertTrue(len(res), len(d) * 2)

    def test_002_taps(self):
        multiple = 10
        overlap = 4

        L = 4
        taps = np.ones(L)
        print overlap, "L = ", L

        # initialize UUT and check results
        self.smt = fbmc.smt_filterbank_rx_cvc(taps, L)

        d = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
        self.src = blocks.vector_source_c(d, vlen=1)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.smt, self.snk)
        self.tb.run()
        np.set_printoptions(linewidth=150)
        # check data
        res = self.snk.data()
        resmatrix = np.array(res).reshape((-1, L)).T
        print resmatrix
        print

        ref = ft.rx(d[: -L//2], taps, L)
        print ref

        self.assertComplexTuplesAlmostEqual(ref.flatten(), resmatrix.flatten())


if __name__ == '__main__':
    gr_unittest.run(qa_smt_filterbank_rx_cvc)
