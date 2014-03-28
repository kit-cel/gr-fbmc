#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014 <+YOU OR YOUR COMPANY+>.
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

class qa_frame_generator_vcvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
    	self.src = blocks.vector_source_c(range(1,17), vlen=2, repeat=False)
    	self.frame_gen = fbmc.frame_generator_vcvc(sym_len=2, frame_len=8)
    	self.snk = blocks.vector_sink_c(vlen=2)
    	self.tb.connect(self.src, self.frame_gen, self.snk)
        self.tb.run ()
        # check data
        ref = (1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0,9,10,11,12,13,14,15,16,0,0,0,0,0,0,0,0)
        data = self.snk.data()
        print data
        print ref
        self.assertComplexTuplesAlmostEqual(ref, data)

if __name__ == '__main__':
    gr_unittest.run(qa_frame_generator_vcvc, "qa_frame_generator_vcvc.xml")
