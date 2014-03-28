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
from gnuradio import blocks, fft
import fbmc_swig as fbmc
import pylab as pl

class qa_tx (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
    	L = 8
    	num_items = L; # does not refer to symbols but to the number of items the head block let's through
    	overlap = 4 # this is hardcoded and not changeable at the moment
    	
    	self.src = blocks.vector_source_c([1.0/pl.sqrt(2)+1.0j/pl.sqrt(2)], vlen=1, repeat=True)
    	self.head = blocks.head(gr.sizeof_gr_complex,num_items)
    	self.s2p = fbmc.serial_to_parallel_cvc(L,L)		
    	self.frame_gen = fbmc.frame_generator_vcvc(L,num_items/L+overlap)
    	self.serialize_iq = fbmc.serialize_iq_vcvc(L)
    	self.apply_betas = fbmc.apply_betas_vcvc(L)
    	self.fft = fft.fft_vcc(L, False, (()), False, 1)
    	self.mult_const = blocks.multiply_const_vcc(([L for i in range(L)]))
    	self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
    	self.output_commutator = fbmc.output_commutator_vcc(L)
    	self.snk = blocks.vector_sink_c(vlen=1)
    	
    	self.tb.connect(self.src, self.head, self.s2p, self.frame_gen)
    	self.tb.connect(self.frame_gen, self.serialize_iq, self.apply_betas)
    	self.tb.connect(self.apply_betas, self.fft, self.mult_const, self.ppfb)
    	self.tb.connect(self.ppfb, self.output_commutator, self.snk)
        self.tb.run ()
        
        # check data
        data = self.snk.data()
        print "len(data):", len(data)
        for i in data:
        	print i

if __name__ == '__main__':
    gr_unittest.run(qa_tx)
