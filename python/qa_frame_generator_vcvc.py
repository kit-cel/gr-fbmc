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
		self.src = blocks.vector_source_c(range(1,9), vlen=2, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=2, num_payload = 2, inverse=0, num_overlap = 4, num_sync = 2) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=2)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run ()
		# check data
		ref = (0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,5,6,7,8,0,0,0,0,0,0,0,0)
		data = self.snk.data()
		#print data
		#print ref
		self.assertComplexTuplesAlmostEqual(ref, data)
	
	def test_002_t (self): # reverse
		print "test 2 - reverse"
		self.src = blocks.vector_source_c(range(1,49), vlen=2, repeat=False)
		self.frame_gen = fbmc.frame_generator_vcvc(sym_len=2, num_payload = 2, inverse=1, num_overlap = 4, num_sync = 2)
		self.snk = blocks.vector_sink_c(vlen=2)
		self.tb.connect(self.src, self.frame_gen, self.snk)
		self.tb.run()
		# check data
		data = self.snk.data()
		ref = (21,22,23,24,45,46,47,48)
		print data
		print ref
		self.assertComplexTuplesAlmostEqual(ref, data)

if __name__ == '__main__':
	gr_unittest.run(qa_frame_generator_vcvc)
