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

class qa_frame_sync_cvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        L = 8
        step_size = 1
        frame_len = 5
        preamble="IAM"
        threshold = 0.8
        num_frames = 2

        #create some input data between [-1,1]
        symbol = random.randn(frame_len*L)
        noise1 = random.randn(frame_len*L)
        noise2 = random.randn(frame_len*L)
        input_data = concatenate((noise1, symbol, symbol, noise2))
        input_data /= max(abs(input_data))

        self.src = blocks.vector_source_c(input_data, vlen=1, repeat=False)
        self.framesync = fbmc.frame_sync_cvc(L=L, frame_len=frame_len, preamble=preamble, step_size=step_size, threshold=threshold)
        self.snk = blocks.vector_sink_c(vlen=L)

        self.tb.connect(self.src, self.framesync, self.snk)

        self.tb.run ()

        # check data
        data = self.snk.data()


if __name__ == '__main__':
    gr_unittest.run(qa_frame_sync_cvc)
