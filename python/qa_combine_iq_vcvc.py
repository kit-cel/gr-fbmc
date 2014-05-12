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

class qa_combine_iq_vcvc (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None

	def test_001_t (self):
		# set up fg
		L = 3
		self.src = blocks.vector_source_c(range(4*L), vlen=L)
		self.comb = fbmc.combine_iq_vcvc(L=L)
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.comb, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		ref = (0+3j, 1+4j, 2+5j, 6+9j, 7+10j, 8+11j)
		self.assertComplexTuplesAlmostEqual(ref, data)
		
	def test_002_t (self):
		# set up fg
		L = 64
		n= L*1000
		self.src = blocks.vector_source_c(range(n), vlen=L)
		self.comb = fbmc.combine_iq_vcvc(L=L)
		self.snk = blocks.vector_sink_c(vlen=L)
		self.tb.connect(self.src, self.comb, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		self.assertEqual(len(data), n/2)
		
if __name__ == '__main__':
	gr_unittest.run(qa_combine_iq_vcvc, "qa_combine_iq_vcvc.xml")
