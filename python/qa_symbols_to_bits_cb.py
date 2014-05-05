#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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
from gnuradio import blocks, digital
import fbmc_swig as fbmc

class qa_symbols_to_bits_cb (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        samples = (1+1j, 1-1j, -1-1j, -1+1j, .5 +1j*.5, .5 -1j*.5, -.5 +1j*.5, -.5 -1j*.5)
        self.src = blocks.vector_source_c(samples)
        self.demod = fbmc.symbols_to_bits_cb()
        self.snk = blocks.vector_sink_b()
        self.tb.connect(self.src, self.demod, self.snk)
        self.tb.run ()
        # check data
        ref = (3,1,0,2,3,1,2,0)
        data = self.snk.data()
        self.assertFloatTuplesAlmostEqual(data, ref)

    def test_002_t (self):
        # set up fg
        n = 100000
        samples = [1 + 1j for i in range(n)]
        self.src = blocks.vector_source_c(samples)
        self.demod = fbmc.symbols_to_bits_cb()
        self.snk = blocks.vector_sink_b()
        self.tb.connect(self.src, self.demod, self.snk)
        self.tb.run ()
        # check data
        data = self.snk.data()
        self.assertEqual(n, len(data))
        
if __name__ == '__main__':
    gr_unittest.run(qa_symbols_to_bits_cb)
