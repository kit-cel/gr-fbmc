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


from gnuradio import gr, gr_unittest
from gnuradio import blocks
import fbmc_swig as fbmc

class qa_frame_generator_vcvc (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None

	def test_001_t (self): # forward
		print "test 1 - forward"
		L=8
		self.src = blocks.vector_source_c(range(1,L+1), vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = 1, inverse=0, num_overlap = 4, num_sync = 3) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run ()
		# check data
		ref = (0,0,0,0,0,0,0,0, 
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   1,2,3,4,5,6,7,8,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0 )
		data = self.snk.data()
		self.assertComplexTuplesAlmostEqual(ref, data)

	def test_002_t (self): # reverse
		print "test 2 - reverse"
		L = 8
		input_data = (0,0,0,0,0,0,0,0, 
			   1,1,-1,-1,1,1,-1,-1,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   1,2,3,4,5,6,7,8)
		self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = 1, inverse=1, num_overlap = 4, num_sync = 3)
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run()
		# check data
		data = self.snk.data()
		ref = (1,2,3,4,5,6,7,8)
		self.assertComplexTuplesAlmostEqual(ref, data)

	def test_003_t (self): # forward
		print "test 3 - forward - many frames"
		L=8
		input_data = range(1,161)
		self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = 1, inverse=0, num_overlap = 4, num_sync = 3) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), len(input_data)*12)

	def test_004_t (self): # reverse
		print "test 4 - reverse - many frames"
		L = 8
		K = 5
		num_frames = 10
		input_data = range(1,(K+11)*L*num_frames+1)
		self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = K, inverse=1, num_overlap = 4, num_sync = 3)
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run()
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), L*K*num_frames)
"""	
	def test_005_t (self): # forward
		print "test 5 - forward - many long frames"
		L=16
		K=222
		num_frames = 1000
		overlap = 4
		sync = 6
		self.src = blocks.vector_source_c(range(1,K*L*num_frames+1), vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = K, inverse=0, num_overlap = overlap, num_sync = sync) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), num_frames*L*(K+sync+2*overlap))	

	def test_006_t (self): # reverse
		print "test 6 - reverse - many long frames"
		L=16
		K=222
		num_frames = 1000
		overlap = 4
		sync = 6
		self.src = blocks.vector_source_c(range((K+2*overlap+sync)*L*num_frames), vlen=L, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L, num_payload = K, inverse=1, num_overlap = overlap, num_sync = sync) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), num_frames*L*K)			
"""
if __name__ == '__main__':
	gr_unittest.run(qa_frame_generator_vcvc)
