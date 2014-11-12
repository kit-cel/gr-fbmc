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


class qa_phydyas_filterbank_rx_cvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_setUp(self):
        # test if flowgraph is set up as expected!
        overlap = 4
        L = 4
        taps = [1,] * (overlap * L + 1)
        phydyas = fbmc.phydyas_filterbank_rx_cvc(taps, L)
        print "overlap: ", phydyas.overlap()
        print "L: ", phydyas.L()
        assert phydyas.overlap() == overlap
        assert phydyas.L() == L

    def test_002_buffers(self):
        print "\ntest002_buffers"
        # test if flowgraph is set up as expected!
        multiple = 4
        overlap = 1
        L = 4
        taps = [1, ] * (overlap * L + 1)
        phydyas = fbmc.phydyas_filterbank_rx_cvc(taps, L)
        print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap()
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
        print len(data)
        src = blocks.vector_source_c(data, vlen=1)
        snk = blocks.vector_sink_c(L)
        tb = gr.top_block()
        tb.connect(src, phydyas, snk)
        tb.run()
        res = snk.data()
        print np.array(res).reshape((-1, L)).T




if __name__ == '__main__':
    gr_unittest.run(qa_phydyas_filterbank_rx_cvc)
