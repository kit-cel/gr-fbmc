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
from gnuradio import blocks, fft
import fbmc_swig as fbmc
import fbmc_test_functions as ft
import numpy as np


class qa_rx_polyphase_cvc(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(linewidth=150, precision=4)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        # test if input-output ratio is correct
        multiple = 100
        L = 16
        overlap = 4
        taps = range(overlap * L + 1)
        d = range(L * multiple)

        # set up fg
        self.src = blocks.vector_source_c(d, vlen=1)
        self.smt = fbmc.rx_polyphase_cvc(taps, L)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.smt, self.snk)
        self.tb.run()

        # check data
        res = self.snk.data()
        self.assertTrue(len(res), len(d) * 2)

    def test_002_commutator(self):
        # check if commutator works correctly.
        multiple = 10
        L = 4
        taps = np.ones(L)

        # initialize UUT and check results
        self.smt = fbmc.rx_polyphase_cvc(taps, L)
        # print "test2 generate data"
        d = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
        self.src = blocks.vector_source_c(d, vlen=1)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.smt, self.snk)
        self.tb.run()

        # check data
        res = self.snk.data()
        resmatrix = np.array(res).reshape((-1, L)).T
        print resmatrix
        print

        ref = ft.rx(d[: -L // 2], taps, L)
        print ref

        self.assertComplexTuplesAlmostEqual(ref.flatten(), resmatrix.flatten())

    def test_003_legacy_small(self):
        multiple = 10
        L = 4
        taps = np.append(np.ones(L), 0.5 * np.ones(L))

        # test data!
        d = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # initialize fg
        smt = fbmc.rx_polyphase_cvc(taps, L)
        self.src = blocks.vector_source_c(d, vlen=1)
        self.snk = blocks.vector_sink_c(vlen=L)

        # old chain
        self.com = fbmc.input_commutator_cvc(L)
        self.pfb = fbmc.polyphase_filterbank_vcvc(L, taps)
        self.fft = fft.fft_vcc(L, False, (), False, 1)
        self.snkold = blocks.vector_sink_c(vlen=L)

        self.tb.connect(self.src, self.com, self.pfb, self.fft, self.snkold)
        self.tb.connect(self.src, smt, self.snk)
        self.tb.run()

        # check data
        res = self.snk.data()
        resmatrix = np.array(res).reshape((-1, L)).T

        old = self.snkold.data()
        oldmatrix = np.array(old).reshape((-1, L)).T

        print np.array(smt.filterbank_taps())
        print "\nresult"
        print resmatrix
        print "\nold"
        print oldmatrix

        self.assertComplexTuplesAlmostEqual(oldmatrix.flatten(), resmatrix.flatten())

    def test_004_legacy_big(self):
        np.set_printoptions(linewidth=150, precision=4)
        multiple = 4
        overlap = 4
        self.cfg = fbmc.fbmc_config(num_used_subcarriers=20, num_payload_sym=16, num_overlap_sym=overlap,
                                    modulation="QPSK", preamble="IAM")
        L = self.cfg.num_total_subcarriers()
        taps = np.array(self.cfg.prototype_taps_float(), dtype=np.float)

        # test data!
        d = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)

        # initialize fg
        smt = fbmc.rx_polyphase_cvc(taps, L)
        self.src = blocks.vector_source_c(d, vlen=1)
        self.snk = blocks.vector_sink_c(vlen=L)

        # old chain
        self.com = fbmc.input_commutator_cvc(L)
        self.pfb = fbmc.polyphase_filterbank_vcvc(L, taps)
        self.fft = fft.fft_vcc(L, False, (), False, 1)
        self.snkold = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.com, self.pfb, self.fft, self.snkold)
        self.tb.connect(self.src, smt, self.snk)
        self.tb.run()

        # check data
        res = self.snk.data()
        resmatrix = np.array(res).reshape((-1, L)).T
        old = self.snkold.data()
        oldmatrix = np.array(old).reshape((-1, L)).T

        print np.array(smt.filterbank_taps())
        print "\nresult"
        print resmatrix
        print "\nold fg"
        print oldmatrix

        self.assertComplexTuplesAlmostEqual(oldmatrix.flatten(), resmatrix.flatten())

    def test_005_python_gen(self):
        # check results against Python implementation
        print "\n\n\n\nlegacy"
        multiple = 10
        overlap = 4
        L = 4
        self.cfg = fbmc.fbmc_config(num_used_subcarriers=20, num_payload_sym=16, num_overlap_sym=overlap,
                                    modulation="QPSK", preamble="IAM")
        L = self.cfg.num_total_subcarriers()
        taps = np.array(self.cfg.prototype_taps_float(), dtype=np.float)
        print "config: overlap =  ", overlap, ", L = ", L

        # generate data and set it for source.
        d = np.arange(1, multiple * L // 2 + 1 + 1, dtype=np.complex)
        self.src = blocks.vector_source_c(d, vlen=1)
        self.smt = fbmc.rx_polyphase_cvc(taps, L)
        self.snk0 = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.smt, self.snk0)
        self.tb.run()

        # same way to generate data in python as in every test!
        ref = ft.rx(d[: -L // 2], taps, L)

        res0 = self.snk0.data()
        resultMatrix = np.array(res0).reshape((-1, L)).T
        print resultMatrix[:, 0:5]
        print np.shape(resultMatrix[:, 0:5])
        print L * resultMatrix[0]
        print "ref", ref[0]
        # sym0 = np.array(res0[L:L * 2])
        #
        # print np.array(res0[0:len(res0) / 2]).reshape((-1, L)).T
        # print
        # print np.array(res1[0:len(res1)/2]).reshape((-1, L)).T
        # for i in range(len(sym0)):
        # print sym0[i], sym1[i]

        # # print sym0
        # # print sym1
        # # print "vector dubidu"
        # # print [sym0.T, sym1.T]
        #
        # print np.shape(ref), np.shape(res0matrix[:, 2:4])
        # print ref
        # print
        # print res0matrix[:, 0:4]

        # self.assertComplexTuplesAlmostEqual(ref.flatten(), res0)
        # self.assertComplexTuplesAlmostEqual(res0, res1)


if __name__ == '__main__':
    gr_unittest.run(qa_rx_polyphase_cvc)
