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
from numpy import *

class qa_input_commutator_cvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        L = 6
        self.src = blocks.vector_source_c(range(1,13), vlen=1)
        self.comm = fbmc.input_commutator_cvc(L)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.comm, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        ref = (1,0,0,1,0,0,4,3,2,4,3,2,7,6,5,7,6,5,10,9,8,10,9,8)
        self.assertComplexTuplesAlmostEqual(data, ref)
        
    def test_002_t (self):
        # prepare reference data
        n = 16
        L = 8
        input_stream = arange(n)
        input_matrix = flipud(r_[zeros(L / 2 - 1), input_stream[:-(L / 2 - 1)]].reshape((L / 2, -1), order='F'))
        input_matrix = r_[input_matrix, input_matrix]
        ref_output = input_matrix.transpose().reshape((n*2,1))

        self.src = blocks.vector_source_c(range(n), vlen=1)
        self.comm = fbmc.input_commutator_cvc(L)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.comm, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertComplexTuplesAlmostEqual(data, ref_output)
        
	def test_003_t (self):
		# test input commutator for large vector lengths
		L = 16
		n = L*1000
		input_stream = arange(n)
		self.src = blocks.vector_source_c(range(n), vlen=1)
        self.comm = fbmc.input_commutator_cvc(L)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.comm, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertTrue(n, len(data))
        
if __name__ == '__main__':
    gr_unittest.run(qa_input_commutator_cvc)
