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

# import sys
# sp = sys.path
# sp = "\n".join(sp)
# print sp

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import fbmc_swig as fbmc
import numpy as np
import fbmc_test_functions as ft
import matplotlib.pyplot as plt


class qa_rx_domain_cvc(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(precision=2, linewidth=150)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_initial(self):
        print "test_001_initial"
        # set up fg
        L = 8
        overlap = 4
        multiple = 4
        taps = np.ones(7, dtype=float)
        print taps
        data = np.arange(1, multiple * L + 1, dtype=np.complex)

        # get blocks for test
        src = blocks.vector_source_c(data, vlen=1)
        rx_domain = fbmc.rx_domain_cvc(taps.tolist(), L)
        snk = blocks.vector_sink_c(vlen=L)

        # connect and run
        self.tb.connect(src, rx_domain, snk)
        self.tb.run()

        # check data
        ref = ft.rx_fdomain(data, taps, L).flatten()
        res = np.array(snk.data())

        # print np.reshape(ref, (-1, L)).T
        # print len(res)
        # print np.reshape(res, (-1, L)).T

        self.assertComplexTuplesAlmostEqual(ref, res, 4)

    def test_002_small_taps(self):
        print "test_002_small_taps"
        # set up fg
        L = 8
        overlap = 4
        multiple = 4
        taps = ft.prototype_fsamples(overlap, False)
        print taps
        data = np.arange(1, multiple * L + 1, dtype=np.complex)
        # get blocks for test
        src = blocks.vector_source_c(data, vlen=1)
        rx_domain = fbmc.rx_domain_cvc(taps.tolist(), L)
        snk = blocks.vector_sink_c(vlen=L)

        # connect and run
        self.tb.connect(src, rx_domain, snk)
        self.tb.run()

        # check data
        ref = ft.rx_fdomain(data, taps, L).flatten()
        res = np.array(snk.data())

        self.assertComplexTuplesAlmostEqual(ref, res, 3)

    def test_003_go_big(self):
        print "test_003_go_big -> try out!"
        # set up fg
        L = 32
        overlap = 4
        multiple = 8
        taps = ft.prototype_fsamples(overlap, False)
        print taps
        # data = np.arange(1, multiple * L + 1, dtype=np.complex)
        data = np.zeros(L, dtype=np.complex)
        data[0] = np.complex(1, 0)
        data[L / 2] = np.complex(1, 0)
        data[L / 4] = np.complex(1, -1)
        data[L / 8] = np.complex(1, 0)
        data[3 * L / 4] = np.complex(1, 1)
        # print data
        # print np.reshape(data, (L, -1))
        data = np.repeat(np.reshape(data, (L, -1)), multiple, axis=1)
        data = data.T.flatten()#np.reshape(data, (L, -1))
        print "shape(data) = ", np.shape(data)
        # print data
        # get blocks for test
        src = blocks.vector_source_c(data, vlen=1)
        rx_domain = fbmc.rx_domain_cvc(taps.tolist(), L)
        impulse_taps = ft.generate_phydyas_filter(L, overlap)
        print "fft_size: ", rx_domain.fft_size()

        snk = blocks.vector_sink_c(vlen=L)
        pfb = fbmc.rx_polyphase_cvc(impulse_taps.tolist(), L)
        snk1 = blocks.vector_sink_c(vlen=L)

        # connect and run
        self.tb.connect(src, rx_domain, snk)
        self.tb.connect(src, pfb, snk1)
        self.tb.run()

        # check data
        ref = ft.rx_fdomain(data, taps, L).T.flatten()
        res = np.array(snk.data())
        res1 = np.array(snk1.data())

        # print "ref: ", np.shape(ref)
        # print "res: ", np.shape(res)
        # print np.reshape(res, (-1, L)).T
        # mat1 = np.reshape(res1, (L, -1)).T
        # print np.shape(mat1)

        self.assertComplexTuplesAlmostEqual(ref, res, 6)

    def test_004_kernel(self):
        L = 32
        overlap = 4
        multiple = 1 * overlap
        taps = ft.prototype_fsamples(overlap, False)
        print taps
        kernel = fbmc.rx_domain_kernel(taps.tolist(), L)
        print "L = ", kernel.L()
        print "overlap = ", kernel.overlap()
        print "fft_size = ", kernel.fft_size()
        print "taps = ", kernel.taps()

        data = np.arange(1, L * multiple + 1, dtype=complex)
        print data
        res = kernel.generic_work_python(data)
        print res


if __name__ == '__main__':
    gr_unittest.run(qa_rx_domain_cvc)
