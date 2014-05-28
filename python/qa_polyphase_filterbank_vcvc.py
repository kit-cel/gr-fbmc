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
        self.cfg = fbmc.fbmc_config(num_used_subcarriers=8, num_payload_sym=18, num_overlap_sym=4, modulation="QPSK", preamble="IAM")
    	num_items = 3;
    	self.src = blocks.vector_source_c([complex(i,i) for i in range(1,self.cfg.num_used_subcarriers()+1)], vlen=self.cfg.num_used_subcarriers(), repeat=True)
    	self.head = blocks.head(gr.sizeof_gr_complex*self.cfg.num_used_subcarriers(),num_items)
    	self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_used_subcarriers(), prototype_taps=self.cfg.prototype_taps())
    	self.snk = blocks.vector_sink_c(vlen=self.cfg.num_used_subcarriers())
    	self.tb.connect(self.src, self.head, self.ppfb, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertEqual(len(data), self.cfg.num_used_subcarriers()*num_items)
        
    def test_002_t (self): # again, just checking the length of the output
        self.cfg = fbmc.fbmc_config(num_used_subcarriers=64, num_payload_sym=18, num_overlap_sym=4, modulation="QPSK", preamble="IAM")
    	num_items = 10000
    	n = self.cfg.num_used_subcarriers()*num_items
    	input_data = range(n)
    	self.src = blocks.vector_source_c(input_data, vlen=self.cfg.num_used_subcarriers(), repeat=False)
    	self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_used_subcarriers(), prototype_taps=self.cfg.prototype_taps())
    	self.snk = blocks.vector_sink_c(vlen=self.cfg.num_used_subcarriers())
    	self.tb.connect(self.src, self.ppfb, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertEqual(len(data), n)       

if __name__ == '__main__':
    gr_unittest.run(qa_polyphase_filterbank_vcvc)
