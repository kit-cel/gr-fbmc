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


class qa_rx_sdft_cvc(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(4, linewidth=150)
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
        phydyas = fbmc.rx_sdft_cvc(taps, L)
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
        taps = np.array([0, 1, 2, 1]) #  np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # set Up flowgraph
        phydyas = fbmc.rx_sdft_cvc(taps, L)
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
        print "\nJohannes test"
        print ref.reshape((-1, L)).T

        ftref = ft.rx(data[: -L // 2], taps, L)

        print "\nFBMC blocks"
        print ftref
        print "\nGR blocks"
        print res.reshape((-1, L)).T

        ftref = ftref.T
        self.assertComplexTuplesAlmostEqual(ftref.flatten(), res.flatten()[2*L:])

    def test_003_filter(self):
        print "\ntest003_filter"
        multiple = 9
        overlap = 2
        L = 4
        taps = np.append([0.0, ], np.ones(overlap * L - 1, dtype=float))
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
        print "len(data) = ", len(data)
        print taps
        print data

        # instatiated blocks and flowgraph
        phydyas = fbmc.rx_sdft_cvc(taps, L)
        print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap(), ", history() = ", phydyas.history()
        src = blocks.vector_source_c(data, vlen=1)
        snk = blocks.vector_sink_c(L)
        tb = gr.top_block()
        tb.connect(src, phydyas, snk)

        # run fg and get results
        tb.run()
        res = np.array(snk.data())
        print "len(res) = ", len(res)

        ref = self.get_reference_output(data, taps, L, overlap, multiple)

        ftref = ft.rx(data[: -L // 2], taps, L)
        print "\nFBMC blocks"
        print ftref
        print "\nGR blocks"
        print res.reshape((-1, L)).T

        ftref = ftref.T
        ftref = ftref.flatten()
        self.assertComplexTuplesAlmostEqual(ftref, res[2 * L: -L])

    def test_004_phydyas_taps(self):
        print "\ntest_004_phydyas_taps"
    # test if flowgraph is set up as expected!
        multiple = 9
        overlap = 4
        L = 32
        taps = ft.generate_phydyas_filter(L, overlap)
        # taps = np.append([0.0, ], np.ones(overlap * L - 1, dtype=float))
        # taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # instatiated blocks and flowgraph
        phydyas = fbmc.rx_sdft_cvc(taps, L)
        print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap()
        src = blocks.vector_source_c(data, vlen=1)
        snk = blocks.vector_sink_c(L)
        tb = gr.top_block()
        tb.connect(src, phydyas, snk)

        # run fg and get results
        tb.run()
        res = np.array(snk.data())

        ref = self.get_reference_output(data, taps, L, overlap, multiple)

        ftref = ft.rx(data[: -L // 2], taps, L)
        print "\nFBMC blocks"
        print np.shape(ftref)
        print ftref

        lcut = 2
        rcut = 5
        print "\nGR blocks"
        print np.shape(res)
        print res[lcut * L: -rcut * L].reshape((-1, L)).T
        # print res.reshape((-1, L)).T

        ftref = ftref.T
        ftref = ftref.flatten()
        self.assertComplexTuplesAlmostEqual(ftref, res[lcut * L: -rcut * L], 3)

    def test_005_legacy(self):
        print "\ntest_005_legacy"
    # test if flowgraph is set up as expected!
        multiple = 9
        overlap = 4
        L = 32
        taps = ft.generate_phydyas_filter(L, overlap)
        data = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # instatiated blocks and flowgraph
        phydyas = fbmc.rx_sdft_cvc(taps, L)
        pfb = fbmc.rx_polyphase_cvc(taps, L)
        print "phydyas: L = ", phydyas.L(), ", overlap = ", phydyas.overlap()
        src = blocks.vector_source_c(data, vlen=1)
        snk0 = blocks.vector_sink_c(L)
        snk1 = blocks.vector_sink_c(L)
        tb = gr.top_block()
        tb.connect(src, phydyas, snk0)
        tb.connect(src, pfb, snk1)

        # run fg and get results
        tb.run()
        res0 = np.array(snk0.data())
        res1 = np.array(snk1.data())

        ref = self.get_reference_output(data, taps, L, overlap, multiple)

        ftref = ft.rx(data[: -L // 2], taps, L)
        print "\nFBMC blocks"
        print ftref

        lcut = 2
        rcut = 5
        print "\nGR blocks"
        print res0[lcut * L: -rcut * L].reshape((-1, L)).T

        print "\nIOTA blocks"
        print res1[8 * L:].reshape((-1, L)).T

        ftref = ftref.T
        ftref = ftref.flatten()
        self.assertComplexTuplesAlmostEqual(ftref, res0[lcut * L: -rcut * L], 3)
        self.assertComplexTuplesAlmostEqual(ftref, res1[8 * L:], 3)

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
            vec = np.flipud(vec)
            ref = np.append(ref, vec)
        return ref


if __name__ == '__main__':
    gr_unittest.run(qa_rx_sdft_cvc)
