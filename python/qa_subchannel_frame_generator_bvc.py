#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2017 <+YOU OR YOUR COMPANY+>.
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
from python_impl import syncalgo, smt_fdomain, filters
import numpy as np

class qa_subchannel_frame_generator_bvc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.data = np.random.randint(0, 1, 5120, dtype=int)
        filter = filters.get_prototype(("PHYDYAS", 256, 4), dtype=self.data.real.dtype)[1:]
        self.sync = syncalgo.syncAlgo(256, filter)

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        self.source = blocks.vector_source_b(self.data)
        self.but = fbmc.subchannel_frame_generator_bvc(256, 5120, 4, self.sync.get_c_sequence("independent"), 1.0, 4,
                                                       range(0, 255, 10))
        self.sink = blocks.vector_sink_c(256)
        self.tb.connect(self.source, self.but)
        self.tb.connect(self.but, self.sink)

        map = self.sync.get_data_map(22, "data")
        matrix = np.empty((256, 22))
        i = 0
        for n in range(256):
            for k in range(22):
                if map[n, k]:
                    matrix[n, k] = self.data[i]
                    i += 1
        matrix = self.sync.insert_preamble(matrix, int_mode="data", mode="independent")
        matrix = self.sync.insert_pilots(matrix, range(0, 255, 10), 4)


        # set up fg
        self.tb.run ()
        # check data
        self.assertComplexTuplesAlmostEqual(self.sink.data(), matrix)

if __name__ == '__main__':
    gr_unittest.run(qa_subchannel_frame_generator_bvc, "qa_subchannel_frame_generator_bvc.xml")
