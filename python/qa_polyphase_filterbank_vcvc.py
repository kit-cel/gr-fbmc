#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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

class qa_polyphase_filterbank_vcvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self): # very basic test
    	vec_len = 8
    	num_items = 3;
    	self.src = blocks.vector_source_c([complex(i,i) for i in range(1,vec_len+1)], vlen=vec_len, repeat=True)
    	self.head = blocks.head(gr.sizeof_gr_complex*vec_len,num_items)
    	self.ppfb = fbmc.polyphase_filterbank_vcvc(L=vec_len)
    	self.snk = blocks.vector_sink_c(vlen=vec_len)
    	self.tb.connect(self.src, self.head, self.ppfb, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertEqual(len(data), vec_len*num_items)

if __name__ == '__main__':
    gr_unittest.run(qa_polyphase_filterbank_vcvc)
