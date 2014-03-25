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

class qa_serialize_iq_vcvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
    	input = [complex(i,i+3) for i in range(1,7)]
    	self.src = blocks.vector_source_c(input, vlen=3)
    	self.ser = fbmc.serialize_iq_vcvc(3)
    	self.snk = blocks.vector_sink_c(vlen=3)
    	self.tb.connect(self.src, self.ser, self.snk)
        # set up fg
        self.tb.run ()
        ref = (1,2,3,4,5,6,4,5,6,7,8,9)
        data = self.snk.data()
        print input
        print ref
        print data
        # check data
        self.assertComplexTuplesAlmostEqual(data, ref)

if __name__ == '__main__':
    gr_unittest.run(qa_serialize_iq_vcvc)
