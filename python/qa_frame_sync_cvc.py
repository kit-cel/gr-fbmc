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
        preamble="IAM"
        threshold = 0.9
        num_frames = 1
        overlap = 4

        # very simple test data
        # this simulates a noisy beginning, 2 consecutive frames, some intermediate noise and again one frame
        # the noise at the end is needed because the frame sync makes use of the forecast function  
        frame = (0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.5)
        noise1 = (0.1,0.2,0.3,0.1)
        input_data = concatenate((noise1, frame, frame, noise1, frame, noise1))
        self.src = blocks.vector_source_c(input_data, vlen=1, repeat=False)
        self.framesync = fbmc.frame_sync_cvc(L=L, frame_len=len(frame), overlap=overlap, preamble=preamble, step_size=step_size, threshold=threshold)
        self.snk = blocks.vector_sink_c(vlen=L)

        self.tb.connect(self.src, self.framesync, self.snk)

        self.tb.run ()

        # check data
        data = self.snk.data()
        print "data:", data
        self.assertComplexTuplesAlmostEqual(data, concatenate((frame,frame,frame)))

    def test_002_t (self):
        # set up fg
        L = 2
        step_size = 1
        preamble="IAM"
        threshold = 0.9
        num_frames = 1
        overlap = 4

        # very simple test data
        pil_symbol = (1,2)
        payl_symbol = (3,4)
        sof_noise = (0.3,0.5,0.2,0.1)

        # this is basically the same as test 1 but with 2 subcarriers instead of one
        frame = concatenate((sof_noise, pil_symbol, pil_symbol, sof_noise, payl_symbol))
        input_data = concatenate((sof_noise, frame, frame, sof_noise, frame, sof_noise, sof_noise, sof_noise))
        
        self.src = blocks.vector_source_c(input_data, vlen=1, repeat=False)
        self.framesync = fbmc.frame_sync_cvc(L=L, frame_len=len(frame)/L, overlap=overlap, preamble=preamble, step_size=step_size, threshold=threshold)
        self.snk = blocks.vector_sink_c(vlen=L)

        self.tb.connect(self.src, self.framesync, self.snk)

        self.tb.run ()

        # check data
        data = self.snk.data()
        print "data:", real(data)
        self.assertComplexTuplesAlmostEqual(data, concatenate((frame,frame,frame)))


if __name__ == '__main__':
    gr_unittest.run(qa_frame_sync_cvc)
