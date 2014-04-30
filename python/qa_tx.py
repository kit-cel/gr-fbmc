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
from gnuradio import digital
import fbmc_swig as fbmc
import pylab as pl
from numpy import *
from time import sleep

class qa_tx (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None
	
	def test_001_t(self):
		print "test 1 - symbol input - M=L - single frame"
		# configuration
		L = 2
		num_payload = 2
		num_sync = 2
		num_overlap = 4
		# random input signal
		input_data = [1+2j, 3+4j, 5+6j, 7+8j]
		
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
		sleep(0.5)
		
		# check data
		output_data = self.snk.data()
		if(len(input_data) != len(output_data)):
			print "input:", input_data
			print "output:", output_data
		self.assertComplexTuplesAlmostEqual(input_data, output_data, 3)
"""		
	def test_002_t(self):
		print "test 2 - symbol input - M<L - single frame"
		# configuration
		M = 3
		L = 4
		num_payload = 2
		num_sync = 2
		num_overlap = 4
		# random input signal
		input_data = [1+2j, 3+4j, 5+6j, 7+8j, 9+10j, 11+12j]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.s2p = fbmc.serial_to_parallel_cvc(len_in=M, vlen_out=L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=0, num_overlap = num_overlap, num_sync = num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(L)
		self.betas = fbmc.apply_betas_vcvc(L=L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
		self.output_commutator = fbmc.output_commutator_vcc(L)
	
		self.tb.connect(self.src, self.s2p, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=L)
		self.inv_fft2 = fft.fft_vcc(L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=1, num_overlap = num_overlap, num_sync = num_sync)
		self.p2s = fbmc.parallel_to_serial_vcc(len_out=M, vlen_in=L)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.p2s, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertComplexTuplesAlmostEqual(input_data, output_data, 3)	

	def test_003_t(self):
		print "test 3 - symbol input - M=L - single long frame"
		# configuration
		L = 2
		num_payload = 202
		num_sync = 2
		num_overlap = 4
		# random input signal
		input_data = range(1,num_payload*L+1)
		
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
		self.assertEqual(len(input_data), len(output_data))
		
	def test_004_t(self):
		print "test 4 - symbol input - M<L - single long frame"
		# configuration
		M = 3
		L = 4
		num_payload = 202
		num_sync = 2
		num_overlap = 4
		# random input signal
		input_data = range(1,M*num_payload+1)
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.s2p = fbmc.serial_to_parallel_cvc(len_in=M, vlen_out=L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=0, num_overlap = num_overlap, num_sync = num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(L)
		self.betas = fbmc.apply_betas_vcvc(L=L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
		self.output_commutator = fbmc.output_commutator_vcc(L)
	
		self.tb.connect(self.src, self.s2p, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=L)
		self.inv_fft2 = fft.fft_vcc(L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=1, num_overlap = num_overlap, num_sync = num_sync)
		self.p2s = fbmc.parallel_to_serial_vcc(len_out=M, vlen_in=L)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.p2s, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data))			

	def test_005_t(self):
		print "test 5 - symbol input - M<L - multiple frames"
		# configuration
		M = 3
		L = 4
		num_payload = 2
		num_sync = 2
		num_overlap = 4
		num_frames = 20
		# random input signal
		#input_data = [1+2j, 3+4j, 5+6j, 7+8j, 9+10j, 11+12j]
		input_data = map(int, random.randint(1, 5, M*num_payload*num_frames))
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.s2p = fbmc.serial_to_parallel_cvc(len_in=M, vlen_out=L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=0, num_overlap = num_overlap, num_sync = num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(L)
		self.betas = fbmc.apply_betas_vcvc(L=L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
		self.output_commutator = fbmc.output_commutator_vcc(L)
	
		self.tb.connect(self.src, self.s2p, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=L)
		self.inv_fft2 = fft.fft_vcc(L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=1, num_overlap = num_overlap, num_sync = num_sync)
		self.p2s = fbmc.parallel_to_serial_vcc(len_out=M, vlen_in=L)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.p2s, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertComplexTuplesAlmostEqual(input_data, output_data, 3)	

		
	def test_06_t(self):
		print "test 6 - byte input"
		# configuration
		M = 3
		L = 4
		num_payload = 22
		num_sync = 2
		num_overlap = 4
		modulation = digital.constellation_qpsk()
		# random input signal
		input_data = map(int, random.randint(0, 4, num_payload*M))
		
		# TX
		self.src = blocks.vector_source_b(input_data, False)
		self.b2s = digital.chunks_to_symbols_bc((modulation.points()), 1)
		self.s2p = fbmc.serial_to_parallel_cvc(len_in=M, vlen_out=L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=0, num_overlap = num_overlap, num_sync = num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(L)
		self.betas = fbmc.apply_betas_vcvc(L=L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
		self.output_commutator = fbmc.output_commutator_vcc(L)
	
		self.tb.connect(self.src, self.b2s, self.s2p, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=L)
		self.inv_fft2 = fft.fft_vcc(L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=L, num_payload = num_payload, inverse=1, num_overlap = num_overlap, num_sync = num_sync)
		self.p2s = fbmc.parallel_to_serial_vcc(len_out=M, vlen_in=L)
		self.s2b = fbmc.symbols_to_bits_cb()
		self.snk = blocks.vector_sink_b(vlen=1)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.p2s, self.s2b, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertFloatTuplesAlmostEqual(input_data, output_data, 3)	
"""			

if __name__ == '__main__':
	gr_unittest.run(qa_tx)
