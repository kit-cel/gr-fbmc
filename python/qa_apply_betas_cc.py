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

class qa_apply_betas_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
    	self.src = blocks.vector_source_c(range(1,21))
    	self.tagged_stream_adaptor = blocks.stream_to_tagged_stream(gr.sizeof_gr_complex, 1, 20, "frame_len")
    	self.apply_betas = fbmc.apply_betas_cc(K=4,M=5)
    	self.snk = blocks.vector_sink_c()
    	self.tb.connect(self.src, self.tagged_stream_adaptor, self.apply_betas, self.snk)
        self.tb.run ()
        data = self.snk.data()
        ref_data = (1, 2j, 3, 4j, 5j, -6, 7j, -8, 9, 10j, 11, 12j, 13j, -14, 15j, -16, 17, 18j, 19, 20j)
        print "len data:", len(data), "len ref_data:", len(ref_data)
        print data[:100]
        self.assertComplexTuplesAlmostEqual(data[:20], ref_data)


if __name__ == '__main__':
    gr_unittest.run(qa_apply_betas_cc)
