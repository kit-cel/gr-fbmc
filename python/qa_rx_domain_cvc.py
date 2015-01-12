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


class qa_rx_domain_cvc(gr_unittest.TestCase):
    def setUp(self):
        np.set_printoptions(precision=2, linewidth=150)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        # set up fg
        taps = np.ones(7)
        L = 8
        overlap = 4
        multiple = 4
        data = np.arange(1, multiple * L + 1)
        print data
        npfft = np.fft.fft(data[0:L * overlap])
        print len(npfft)
        print npfft
        npfft = np.roll(npfft, -overlap)
        ref = []
        for i in range(L):
            part = npfft[0:7]
            print part
            dotprod = np.dot(part, taps)
            print dotprod
            ref.append(dotprod)
            npfft = np.roll(npfft, overlap)
        ref = np.array(ref)
        print ref

        src = blocks.vector_source_c(data, vlen=1)
        rx_domain = fbmc.rx_domain_cvc(taps, L, overlap)
        snk = blocks.vector_sink_c(vlen=L)

        self.tb.connect(src, rx_domain, snk)

        self.tb.run()

        # check data
        res = np.array(snk.data())
        print len(res)
        print res


if __name__ == '__main__':
    gr_unittest.run(qa_rx_domain_cvc)
