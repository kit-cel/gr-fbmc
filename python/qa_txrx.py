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
		return self.cfg.num_total_subcarriers()*self.cfg.num_sym_frame()

	def setUp (self):
		self.tb = gr.top_block ()
		self.cfg = fbmc.fbmc_config(num_used_subcarriers=16, num_payload_sym=18, num_overlap_sym=4, modulation="QPSK", preamble="IAM")
		
		# default configuration, can be overwritten in the test
		self.num_frames = 100
			
		
		# prepare all blocks needed for a full TXRX chain but connect only the needed ones
		
		# TX 
		self.b2s = digital.chunks_to_symbols_bc(self.cfg.constellation_points(), 1)
		self.serial_to_parallel = fbmc.serial_to_parallel_cvc(self.cfg.num_used_subcarriers(), self.cfg.num_total_subcarriers())
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=self.cfg.num_total_subcarriers(), num_payload = self.cfg.num_payload_sym(), inverse=0, num_overlap = self.cfg.num_overlap_sym(), num_sync = self.cfg.num_sync_sym())
		self.preamble_insertion = fbmc.preamble_insertion_vcvc(L=self.cfg.num_total_subcarriers(), frame_len = self.cfg.num_payload_sym()+self.cfg.num_sync_sym()+2*self.cfg.num_overlap_sym(), type=self.cfg.preamble(), overlap=self.cfg.num_overlap_sym())
		self.oqam = fbmc.serialize_iq_vcvc(self.cfg.num_total_subcarriers())
		self.betas = fbmc.apply_betas_vcvc(L=self.cfg.num_total_subcarriers(), inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(self.cfg.num_total_subcarriers(), False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_used_subcarriers(), prototype_taps=self.cfg.prototype_taps())
		self.output_commutator = fbmc.output_commutator_vcc(self.cfg.num_total_subcarriers())	
		
		# RX path
		self.frame_sync = fbmc.frame_sync_cc(self.cfg.num_used_subcarriers(), self.cfg.num_sym_frame(), self.cfg.num_overlap_sym(), "IAM", 1, 0.999)
		self.input_commutator = fbmc.input_commutator_cvc(self.cfg.num_total_subcarriers())
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_used_subcarriers(), prototype_taps=self.cfg.prototype_taps())
		self.inv_fft2 = fft.fft_vcc(self.cfg.num_total_subcarriers(), False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=self.cfg.num_total_subcarriers(), inverse=1)
		self.qam = fbmc.combine_iq_vcvc(self.cfg.num_total_subcarriers())
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=self.cfg.num_total_subcarriers(), num_payload = self.cfg.num_payload_sym(), inverse=1, num_overlap = self.cfg.num_overlap_sym(), num_sync = self.cfg.num_sync_sym())		
		self.parallel_to_serial = fbmc.parallel_to_serial_vcc(self.cfg.num_used_subcarriers(), self.cfg.num_total_subcarriers())
		self.s2b = fbmc.symbols_to_bits_cb(self.cfg.constellation())
				
	def tearDown (self):
		self.tb = None
		

	def test_001_t(self):
		print "test 1 - src to parallelization"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.cfg.num_total_subcarriers())]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames)
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.cfg.num_payload_sym()*self.cfg.num_total_subcarriers())
	
	def test_004_t(self):
		print "test 4 - serializer and OQAM without frame generator inbetween"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.cfg.num_total_subcarriers())]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames) 
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.oqam, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*len(input_data)*2)	
		
	def test_005_t(self):
		print "test 5 - serializer, OQAM, betas"
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))	
		
	def test_006_t(self):
		print "test 6 - serializer, OQAM, betas, ifft"
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.inv_fft, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))				
		
	def test_007_t(self):
		print "test 7 - serializer, OQAM, betas, ifft, ppfb"
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		
		# TX
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		self.tb.connect(self.src, self.serial_to_parallel, self.oqam, self.betas, self.inv_fft, self.ppfb, self.snk)
			  
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	

		self.assertEqual(len(input_data)*2, len(output_data))	
		
	def test_008_t(self):		
		print "test 8 - serializer, OQAM, betas, ifft, ppfb, output commutator"
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		
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
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.cfg.num_total_subcarriers())]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames)
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.frame_gen, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.get_frame_len())
			
	def test_003_t(self):
		print "test 3 - src to serializer"
		
		# random input signal
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.cfg.num_total_subcarriers())]		
		
		self.src = blocks.vector_source_c(input_data, vlen=1, repeat=True) 
		self.head = blocks.head(gr.sizeof_gr_complex, len(input_data)*self.num_frames) 
		self.snk = blocks.vector_sink_c(vlen=self.cfg.num_total_subcarriers())
		
		self.tb.connect(self.src, self.head, self.serial_to_parallel, self.frame_gen, self.oqam, self.snk)
		self.tb.run()
		
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), self.num_frames*self.get_frame_len()*2)		
		
	def test_010_t(self):
		print "test 10 - complete tx chain (with frame generator)"
		
		
		input_data = [i+1j*i for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		self.src = blocks.vector_source_c(input_data, vlen=1)
		self.snk = blocks.vector_sink_c(vlen=1)
		
		# connect and run
		self.tb.connect(self.src, self.serial_to_parallel, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator, self.snk)
		self.tb.run()
		
		# check
		output_data = self.snk.data()
		self.assertEqual(len(input_data), len(output_data) - self.num_frames*self.cfg.num_total_subcarriers()*(2*self.cfg.num_overlap_sym()+self.cfg.num_sync_sym()))
		
	def test_009_t(self):
		print "test 9 - symbol input - M=L - whole TXRX chain"

		# random input signal
		input_data = [sin(i)+1j*cos(i) for i in range(self.cfg.num_payload_sym()*self.num_frames*self.cfg.num_total_subcarriers())]
		
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
		self.assertComplexTuplesAlmostEqual(output_data, input_data, 2)
			
	
	def test_011_t(self):
		print "test 11 - bit input - M<L - whole TXRX chain"
		# configuration
		self.cfg = fbmc.fbmc_config(num_used_subcarriers = 5, num_payload_sym = 18, modulation="QPSK", preamble="IAM")

		# random input signal
		input_data = map(int, random.randint(0, 4, self.cfg.num_used_subcarriers()*self.num_frames*self.cfg.num_payload_sym()))
		
		# TX
		self.src = blocks.vector_source_b(input_data, False)
		self.b2s = digital.chunks_to_symbols_bc((self.cfg.constellation_points()), 1)
		self.s2p = fbmc.serial_to_parallel_cvc(len_in=self.cfg.num_used_subcarriers(), vlen_out=self.cfg.num_total_subcarriers())
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=self.cfg.num_total_subcarriers(), num_payload = self.cfg.num_payload_sym(), inverse=0, num_overlap = self.cfg.num_overlap_sym(), num_sync = self.cfg.num_sync_sym())
		
		self.oqam = fbmc.serialize_iq_vcvc(self.cfg.num_total_subcarriers())
		self.betas = fbmc.apply_betas_vcvc(L=self.cfg.num_total_subcarriers(), inverse=0)
		from gnuradio import fft # why does the import at the top not work??
		self.inv_fft = fft.fft_vcc(self.cfg.num_total_subcarriers(), False, (()), False, 1)
		self.ppfb = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_total_subcarriers(), prototype_taps=self.cfg.prototype_taps())
		self.output_commutator = fbmc.output_commutator_vcc(self.cfg.num_total_subcarriers())
	
		self.tb.connect(self.src, self.b2s, self.s2p, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.input_commutator = fbmc.input_commutator_cvc(self.cfg.num_total_subcarriers())
		self.ppfb2 = fbmc.polyphase_filterbank_vcvc(L=self.cfg.num_total_subcarriers(), prototype_taps=self.cfg.prototype_taps())
		self.inv_fft2 = fft.fft_vcc(self.cfg.num_total_subcarriers(), False, (()), False, 1)
		self.betas2 = fbmc.apply_betas_vcvc(L=self.cfg.num_total_subcarriers(), inverse=1)
		self.qam = fbmc.combine_iq_vcvc(self.cfg.num_total_subcarriers())
		self.frame_gen2 = fbmc.frame_generator_vcvc(sym_len=self.cfg.num_total_subcarriers(), num_payload = self.cfg.num_payload_sym(), inverse=1, num_overlap = self.cfg.num_overlap_sym(), num_sync = self.cfg.num_sync_sym())
		self.p2s = fbmc.parallel_to_serial_vcc(len_out=self.cfg.num_used_subcarriers(), vlen_in=self.cfg.num_total_subcarriers())
		self.s2b = fbmc.symbols_to_bits_cb(self.cfg.constellation())
		self.snk = blocks.vector_sink_b(vlen=1)
		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self.inv_fft2, self.betas2, self.qam, self.frame_gen2, self.p2s, self.s2b, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()
		self.assertComplexTuplesAlmostEqual(input_data, output_data)	

	def test_012_t(self):
		print "test 12 - bit input - whole TXRX chain"

		# random input signal
		input_data = map(int, random.randint(0, 4, self.cfg.num_used_subcarriers()*self.num_frames*self.cfg.num_payload_sym()))		
		# TX
		self.src = blocks.vector_source_b(input_data, False)
		self.tb.connect(self.src, self.b2s, self.serial_to_parallel, self.frame_gen, self.oqam, self.betas, self.inv_fft, self.ppfb, self.output_commutator)
			  
		# RX
		self.snk = blocks.vector_sink_b(vlen=1)		
		self.tb.connect(self.output_commutator, self.input_commutator, self.ppfb2, self. inv_fft2, self.betas2, self.qam, self. frame_gen2, self.parallel_to_serial, self.s2b, self.snk)
		
		# run the flow graph
		self.tb.run()
		
		# check data
		output_data = self.snk.data()	
		self.assertFloatTuplesAlmostEqual(output_data, input_data[:len(output_data)])

if __name__ == '__main__':
	gr_unittest.run(qa_tx)
