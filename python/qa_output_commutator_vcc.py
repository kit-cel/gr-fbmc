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
from numpy import *
import fbmc_swig as fbmc

class qa_output_commutator_vcc (gr_unittest.TestCase):

	def setUp (self):
		self.tb = gr.top_block ()

	def tearDown (self):
		self.tb = None

	def test_001_t (self):
		self.src = blocks.vector_source_c([complex(i,i) for i in range(1,5)], vlen=4)
		self.comm = fbmc.output_commutator_vcc(L=4)
		self.snk = blocks.vector_sink_c(vlen=1)
		self.tb.connect(self.src, self.comm, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		ref = (4+4j, 6+6j)
		self.assertComplexTuplesAlmostEqual(data, ref)
		
	def test_002_t (self):
		# test for large vector lengths
		L = 16
		n = L*1000
		self.src = blocks.vector_source_c(range(n), vlen=L)
		self.comm = fbmc.output_commutator_vcc(L=L)
		self.snk = blocks.vector_sink_c(vlen=1)
		self.tb.connect(self.src, self.comm, self.snk)
		self.tb.run ()
		# check data
		data = self.snk.data()
		self.assertEqual(n/2, len(data))

if __name__ == '__main__':
	gr_unittest.run(qa_output_commutator_vcc)
