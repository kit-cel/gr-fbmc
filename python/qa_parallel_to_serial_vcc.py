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

class qa_parallel_to_serial_vcc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        self.src = blocks.vector_source_c(range(10), vlen=5)
        self.p2s = fbmc.parallel_to_serial_vcc(3,5)
        self.snk = blocks.vector_sink_c()
        self.tb.connect(self.src, self.p2s, self.snk)
        self.tb.run ()
        # check data
        ref = (0,1,2,5,6,7)
        data = self.snk.data()
        self.assertComplexTuplesAlmostEqual(ref, data)
        
    def test_002_t (self):
        # set up fg
        self.src = blocks.vector_source_c(range(10), vlen=5)
        self.p2s = fbmc.parallel_to_serial_vcc(5,5)
        self.snk = blocks.vector_sink_c()
        self.tb.connect(self.src, self.p2s, self.snk)
        self.tb.run ()
        # check data
        ref = range(10)
        data = self.snk.data()
        self.assertComplexTuplesAlmostEqual(ref, data)		

if __name__ == '__main__':
    gr_unittest.run(qa_parallel_to_serial_vcc)
