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

class qa_rx (gr_unittest.TestCase):
	
	def get_frame_len(self):
		return self.L*(self.num_payload + 2*self.num_overlap + self.num_sync)

	def setUp (self):
		self.tb = gr.top_block ()
		
		# default configuration, can be overwritten in the test
		self.L = 16
		self.num_payload = 222
		self.num_sync = 22
		self.num_overlap = 4
		self.num_frames = 100		
		
		# RX path
		self.input_commutator = fbmc.input_commutator_cvc(self.L)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.L)
		self.inv_fft = fft.fft_vcc(self.L, False, (()), False, 1)
		self.betas = fbmc.apply_betas_vcvc(L=self.L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(self.L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=self.L, num_payload = self.num_payload, inverse=1, num_overlap = self.num_overlap, num_sync = self.num_sync)		
		self.parallel_to_serial = fbmc.parallel_to_serial_vcc(self.L, self.L)
		self.symbols_to_bits = fbmc.symbols_to_bits_cb()
		
	def tearDown (self):
		self.tb = None
	
	def test_001_t (self):
		print "test 1 - input commutator"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.input_commutator, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data)*2, len(output_data))
		
	def test_002_t (self):
		print "test 2 - ppfb"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data))
		
	def test_003_t (self):
		print "test 3 - ppfb, ifft"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data))		

	def test_004_t (self):
		print "test 4 - ppfb, ifft, betas"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.betas, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data))		
		
	def test_005_t (self):
		print "test 5 - ppfb, ifft, betas, qam"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.betas, self.qam, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data)*2)	
				
	def test_006_t (self):
		print "test 6 - ppfb, ifft, betas, qam, frame generator"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.betas, self.qam, self.frame_gen, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data)*2 + self.L*self.num_frames*(self.num_overlap*2 + self.num_sync))	
		
	def test_007_t (self):
		print "test 7 - ppfb, ifft, betas, qam, frame generator, parallel to serial"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.betas, self.qam, self.frame_gen, self.parallel_to_serial, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data)*2 + self.L*self.num_frames*(self.num_overlap*2 + self.num_sync))			

	def test_008_t (self):
		print "test 8 - ppfb, ifft, betas, qam, frame generator, parallel to serial, QAM to bits"
		
		# random input data
		input_data = range(self.get_frame_len()*self.num_frames)
		self.src = blocks.vector_source_c(input_data, vlen=self.L)
		self.snk = blocks.vector_sink_b(vlen=1)
		
		# connect and run
		self.tb.connect(self.src, self.ppfb, self.inv_fft, self.betas, self.qam, self.frame_gen, self.parallel_to_serial, self.symbols_to_bits, self.snk)
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data)*2 + self.L*self.num_frames*(self.num_overlap*2 + self.num_sync))
						
if __name__ == '__main__':
	gr_unittest.run(qa_rx)
