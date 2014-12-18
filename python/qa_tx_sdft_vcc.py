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
import numpy as np


class qa_tx_sdft_vcc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_setup(self):
        multiple = 6
        overlap = 1  # must be 1 in this test!
        L = 4
        taps = np.array([0, 1, 2, 1]) #  np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L + 1, dtype=np.complex)

        try:  # make sure ctor checks work correctly!
            dummy = fbmc.tx_sdft_vcc([1, 1, 1], L)
        except RuntimeError as e:
            s = str(e)
            pos = s.find("number of filter taps must be equal to")
            if pos < 0:
                raise

        tx_sdft = fbmc.tx_sdft_vcc(taps, L)
        src = blocks.vector_source_c(data, vlen=L)
        snk = blocks.vector_sink_c(vlen=1)
        # set up fg
        self.tb.connect(src, tx_sdft, snk)

        self.tb.run()
        # check data

        print "L: ", tx_sdft.L()
        print "overlap: ", tx_sdft.overlap()
        print "taps: ", tx_sdft.taps()

        res = snk.data()
        print res

    def test_002_crunch(self):
        print "\n\n\ntest_002"
        multiple = 6
        overlap = 1
        L = 4
        taps = np.array([1, 1, 1, 1, 2, 2, 2, 2, ]) #  np.ones(overlap * L, dtype=float)
        taps = np.append(taps, [0.0, ])
        data = np.arange(1, multiple * L + 1, dtype=np.complex)

        tx_sdft = fbmc.tx_sdft_vcc(taps, L)
        src = blocks.vector_source_c(data, vlen=L)
        snk0 = blocks.vector_sink_c(vlen=1)

        ft = fft.fft_vcc(L, False, (), False, 1)
        pfb = fbmc.polyphase_filterbank_vcvc(L, taps)
        comm = fbmc.output_commutator_vcc(L=L)
        snk1 = blocks.vector_sink_c(vlen=1)
        self.tb.connect(src, tx_sdft, snk0)
        self.tb.connect(src, ft, pfb, comm, snk1)

        self.tb.run()
        r0 = np.array(snk0.data())
        r1 = np.array(snk1.data())

        # print "len(data) = ", len(data)
        # print "len(r0) = ", len(r0)
        # print "len(r1) = ", len(r1)
        pfb_taps = np.array(pfb.filter_branch_taps())
        # print "\nPFB taps"
        # print pfb_taps
        #
        # print "\ndata"
        # print data.reshape((-1, L)).T

        print "\ntx_sdft"
        print r0.reshape((-1, L)).T

        print "\npolyphase"
        print r1.reshape((-1, L)).T

        tmp = np.concatenate((r0, r1)).reshape((2, -1)).T
        for x, y in tmp:
            print x, y, "\tis equal", x==y

        self.assertComplexTuplesAlmostEqual(r0, r1)

    def test_003_go_big(self):
        print "\n\n\ntest_003"
        multiple = 20
        overlap = 4

        self.cfg = fbmc.fbmc_config(num_used_subcarriers=20, num_payload_sym=16, num_overlap_sym=overlap,
                                    modulation="QPSK", preamble="IAM")
        L = self.cfg.num_total_subcarriers()
        taps = np.array(self.cfg.prototype_taps_float(), dtype=np.float)
        taps[-1] = 0

        data = np.arange(1, L + 1, dtype=np.complex)
        data = np.repeat(data, multiple)

        tx_sdft = fbmc.tx_sdft_vcc(taps, L)
        src = blocks.vector_source_c(data, vlen=L)
        snk0 = blocks.vector_sink_c(vlen=1)

        ft = fft.fft_vcc(L, False, (), False, 1)
        pfb = fbmc.polyphase_filterbank_vcvc(L, taps)
        comm = fbmc.output_commutator_vcc(L=L)
        snk1 = blocks.vector_sink_c(vlen=1)
        self.tb.connect(src, tx_sdft, snk0)
        self.tb.connect(src, ft, pfb, comm, snk1)

        self.tb.run()
        r0 = np.array(snk0.data())
        r1 = np.array(snk1.data())

        # print "len(data) = ", len(data)
        # print "len(r0) = ", len(r0)
        # print "len(r1) = ", len(r1)
        pfb_taps = np.array(pfb.filter_branch_taps())
        # print "\nPFB taps"
        # print pfb_taps
        #
        # print "\ndata"
        # print data.reshape((-1, L)).T

        print "\ntx_sdft"
        print r0.reshape((-1, L)).T

        print "\npolyphase"
        print r1.reshape((-1, L)).T

        # tmp = np.concatenate((r0, r1)).reshape((2, -1)).T
        # for x, y in tmp:
        #     print x, y, "\tis equal", x==y

        self.assertComplexTuplesAlmostEqual(r0, r1, 4)

if __name__ == '__main__':
    gr_unittest.run(qa_tx_sdft_vcc)
