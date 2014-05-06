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
from numpy import *

class qa_tx (gr_unittest.TestCase):
	
	def get_frame_len(self):
		return self.L*(self.num_payload + 2*self.num_overlap + self.num_sync)

	def setUp (self):
		self.tb = gr.top_block ()
		
		# default configuration, can be overwritten in the test
		self.L = 16
		self.num_payload = 22
		self.num_sync = 2
		self.num_overlap = 4
		self.num_frames = 100
			
		
		# prepare all blocks needed for a full TXRX chain but connect only the needed ones
		
		# TX 
		self.serial_to_parallel = fbmc.serial_to_parallel_cvc(self.L, self.L)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=self.L, num_payload = self.num_payload, inverse=0, num_overlap = self.num_overlap, num_sync = self.num_sync)
		self.oqam = fbmc.serialize_iq_vcvc(self.L)
		self.betas = fbmc.apply_betas_vcvc(L=self.L, inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(self.L, False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.L)
		self.output_commutator = fbmc.output_commutator_vcc(self.L)	
		
		# RX path
		self.input_commutator = fbmc.input_commutator_cvc(self.L)
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=self.L)
		self.inv_fft2 = fft.fft_vcc(self.L, False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=self.L, inverse=1)
		self.qam = fbmc.combine_iq_vcvc(self.L)
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=self.L, num_payload = self.num_payload, inverse=1, num_overlap = self.num_overlap, num_sync = self.num_sync)		
		self.parallel_to_serial = fbmc.parallel_to_serial_vcc(self.L, self.L)
				
	def tearDown (self):
		self.tb = None
		

	def test_001_t(self):
		print "test 1 - src to parallelization"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.L)]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.num_payload*self.L)
	
	def test_004_t(self):
		print "test 4 - serializer and OQAM without frame generator inbetween"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.L)]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames) 
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.oqam, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*len(input_data)*2)	
		
	def test_005_t(self):
		print "test 5 - serializer, OQAM, betas"
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))	
		
	def test_006_t(self):
		print "test 6 - serializer, OQAM, betas, ifft"
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.inv_fft, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))				
		
	def test_007_t(self):
		print "test 7 - serializer, OQAM, betas, ifft, ppfb"
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.inv_fft, self.ppfb, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))	
		
	def test_008_t(self):		
		print "test 8 - serializer, OQAM, betas, ifft, ppfb, output commutator"
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=1)
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data), len(output_data))			
		
	
	
	def test_002_t(self):
		print "test 2 - src to frame generator"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.L)]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames)
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.frame_gen, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.get_frame_len())
			
	def test_003_t(self):
		print "test 3 - src to serializer"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.L)]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames) 
		self.snk = blocks.vector_sink_c(vlen=self.L)
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.frame_gen, self.oqam, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.get_frame_len()*2)		
		
	def test_010_t(self):
		print "test 10 - complete tx chain (with frame generator)"
		
		
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		# connect and run
		self.tb.connect(self.src, self.serial_to_parallel, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator, self.snk)
		self.tb.run()
		
		# check
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data) - self.num_frames*self.L*(2*self.num_overlap+self.num_sync))
		
"""				
	def test_009_t(self):
		print "test 9 - symbol input - M=L - single frame - whole TXRX chain"

		# random input signal
		input_data = [i+1j*i for i in range(self.num_payload*self.num_frames*self.L)]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.tb.connect(self.src, self.serial_to_parallel, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.snk = blocks.vector_sink_c(vlen=1)		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self. inv_fft2, self.betas2, self.qam, self. frame_gen2, self.parallel_to_serial, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	
		#if (len(input_data) != len(output_data)):
		#	print "output:", output_data
		self.assertComplexTuplesAlmostEqual(input_data, output_data, 3)	
	
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
