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
from gnuradio import fft
import fbmc_swig as fbmc
import pylab as pl
from numpy import *

class qa_tx (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None
	
	def test_001_t(self):
		# configuration
		L = 2
		num_payload = 2
		num_sync = 2
		num_overlap = 4
		# random input signal
		input_data = [1+2j, 3+4j, 5+6j, 7+8j]
		
		print input_data
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=0, num_overlap = num_overlap, num_sync = num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(L)
		self.betas = fbmc.apply_betas_vcvc(L=L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
		self.output_commutator = fbmc.output_commutator_vcc(L)
	
		self.tb.connect(self.src, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=L)
		self.inv_fft2 = fft.fft_vcc(L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=1, num_overlap = num_overlap, num_sync = num_sync)
		self.snk = blocks.vector_sink_c(vlen=L)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertComplexTuplesAlmostEqual(input_data, output_data, 3)

if __name__ == '__main__':
	gr_unittest.run(qa_tx)
