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

class qa_preamble_insertion_vcvc (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None

	def test_001_t (self):
		# insert preamble into two consecutive frames
		L=8
		input_data = (1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   1,2,3,4,5,6,7,8,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   1,2,3,4,5,6,7,8,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   1,2,3,4,5,6,7,8,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0,
					   0,0,0,0,0,0,0,0 )
		self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
		self.preamble_insertion = fbmc.preamble_insertion_vcvc(L=L, frame_len = 15, type="IAM2", overlap=4) # the frame len includes the length of the overlap
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.preamble_insertion, self.snk)
		self.tb.run ()
		# check data
		ref = (1,1,-1,-1,1,1,-1,-1, 
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1, 
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   1,2,3,4,5,6,7,8,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   1,1,-1,-1,1,1,-1,-1, 
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1, 
			   1,1,-1,-1,1,1,-1,-1,
			   1,1,-1,-1,1,1,-1,-1,
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


if __name__ == '__main__':
	gr_unittest.run(qa_preamble_insertion_vcvc)
