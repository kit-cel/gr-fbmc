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

class qa_apply_betas_vcvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
		# set up fg
		self.src = blocks.vector_source_c(range(1,41), repeat=False, vlen=8)
		self.apply_betas = fbmc.apply_betas_vcvc(L=8)
		self.snk = blocks.vector_sink_c(vlen=8)
		self.tb.connect(self.src, self.apply_betas, self.snk)
		self.tb.run ()
		# test data      
		data = self.snk.data()
		ref_data = (1, 2j, -3, -4j, 5, 6j, -7, -8j, 9j, 10, -11j, -12, 13j, 14, -15j, -16, -17, -18j, 19, 20j, -21, -22j, 23, 24j, -25j, -26, 27j, 28, -29j, -30, 31j, 32, 33, 34j, -35, -36j, 37, 38j, -39, -40j)
		self.assertComplexTuplesAlmostEqual(data, ref_data)

if __name__ == '__main__':
    gr_unittest.run(qa_apply_betas_vcvc)
