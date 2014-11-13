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
import fbmc_test_functions as ft


class qa_phydyas_filterbank_rx_cvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_setUp(self):
        print "test_001_setUp"
        # test if flowgraph is set up as expected!
        # does not throw any runtime errors unexpectedly!
        overlap = 4
        L = 4
        taps = np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        phydyas = fbmc.phydyas_filterbank_rx_cvc(taps, L)
        print "overlap: ", phydyas.overlap()
        print "L: ", phydyas.L()
        assert phydyas.overlap() == overlap
        assert phydyas.L() == L

    def test_002_buffers(self):
        print "\ntest002_buffers"
        # test if fg runs as expected on a minimum example!
        multiple = 5
        overlap = 1  # must be 1 in this test!
        L = 4
        taps = np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # set Up flowgraph
        phydyas = fbmc.phydyas_filterbank_rx_cvc(taps, L)
        print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap()
        src = blocks.vector_source_c(data, vlen=1)
        snk = blocks.vector_sink_c(L)
        tb = gr.top_block()
        tb.connect(src, phydyas, snk)

        # run fg and get results
        tb.run()
        res = np.array(snk.data())

        # check results
        ref = self.get_reference_output(data, taps, L, overlap, multiple)
        print
        print ref.reshape((-1, L)).T

        ftref = ft.rx(data[: -L // 2], taps, L)

        print
        print ftref.reshape((-1, L)).T
        print
        print res.reshape((-1, L)).T

        # self.assertComplexTuplesAlmostEqual(ref.flatten(), res.flatten())

    # def test_003_filter(self):
    #     print "\ntest003_filter"
    #     # test if flowgraph is set up as expected!
    #     multiple = 5
    #     overlap = 2
    #     L = 4
    #     taps = np.ones(overlap * L, dtype=float)
    #     taps = np.append(taps, [0.0, ])
    #     data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
    #
    #     # instatiated blocks and flowgraph
    #     phydyas = fbmc.phydyas_filterbank_rx_cvc(taps, L)
    #     print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap()
    #     src = blocks.vector_source_c(data, vlen=1)
    #     snk = blocks.vector_sink_c(L)
    #     tb = gr.top_block()
    #     tb.connect(src, phydyas, snk)
    #
    #     # run fg and get results
    #     tb.run()
    #     res = np.array(snk.data())
    #
    #     ref = self.get_reference_output(data, taps, L, overlap, multiple)
    #     print
    #     print ref.reshape((-1, L)).T
    #     print
    #     print res.reshape((-1, L)).T
    #
    #     self.assertComplexTuplesAlmostEqual(ref.flatten(), res.flatten())

    def get_reference_output(self, data, taps, L, overlap, multiple):
        data = np.append([0, ] * (L - 1), data)  # prepend '0's to match history!
        taps = taps[0:-1]  # remove last tap just like in the block
        taps = taps[::-1]  # bring 'em in correct order
        ref = []
        for i in range(multiple):
            pos = i * L // 2
            vec = data[pos:pos + (L * overlap)]
            if len(vec) != len(taps):
                break
            vec = np.multiply(vec, taps)
            vec = vec.reshape([overlap, -1])
            vec = np.sum(vec, axis=0)
            ref = np.append(ref, vec)
        return ref




if __name__ == '__main__':
    gr_unittest.run(qa_phydyas_filterbank_rx_cvc)
