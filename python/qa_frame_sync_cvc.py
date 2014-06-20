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
from numpy import *
from matplotlib import pyplot

class qa_frame_sync_cvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        L = 1
        step_size = 1
        frame_len = 7
        preamble="IAM"
        threshold = 0.9
        num_frames = 1
        overlap = 4

        #create some input data between [-1,1]
        #pil_symbol = random.randn(L)
        #payl_symbol = random.randn(L)
        #sof_noise = random.randn(L*overlap/2)
        #noise1 = random.randn(frame_len*L)
        #noise2 = random.randn(frame_len*L)

        # very simple test data
        pil_symbol = ones((L,1),dtype=complex64)
        payl_symbol = ones((L,1),dtype=complex64)/2
        sof_noise = zeros((L*overlap/2,1),dtype=complex64)

        # this simulates a perfect frame
        input_data = concatenate((sof_noise, pil_symbol, pil_symbol, sof_noise, payl_symbol))
        print "input:", input_data
        
        input_data = (0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.5)
        self.src = blocks.vector_source_c(input_data, vlen=1, repeat=False)
        self.framesync = fbmc.frame_sync_cvc(L=L, frame_len=frame_len, overlap=overlap, preamble=preamble, step_size=step_size, threshold=threshold)
        self.snk = blocks.vector_sink_c(vlen=L)

        self.tb.connect(self.src, self.framesync, self.snk)

        self.tb.run ()

        # check data
        data = self.snk.data()
        print "data:", data
        self.assertComplexTuplesAlmostEqual(data, input_data)


if __name__ == '__main__':
    gr_unittest.run(qa_frame_sync_cvc)
