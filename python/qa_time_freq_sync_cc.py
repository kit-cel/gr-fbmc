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
import numpy as np
import fbmc_swig as fbmc

class qa_time_freq_sync_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        L = 6
        self.src = blocks.vector_source_c(np.arange(L/2)+1, repeat=True)
        self.head = blocks.head(gr.sizeof_gr_complex, L*10)
        self.tfsync = fbmc.time_freq_sync_cc(L, 0.99, 10, L/2, 0)
        self.snk = blocks.vector_sink_c()

        self.tb.connect(self.src, self.head, self.tfsync, self.snk)
        self.tb.run ()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_time_freq_sync_cc)
