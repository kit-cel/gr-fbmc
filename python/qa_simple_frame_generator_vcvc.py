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


from gnuradio import gr, gr_unittest
from gnuradio import blocks
import fbmc_swig as fbmc
import numpy as np


class qa_simple_frame_generator_vcvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):  # forward
        np.set_printoptions(precision=2, linewidth=150)
        print "test 1 - forward"
        L = 8
        self.src = blocks.vector_source_c(range(1, L + 1), vlen=L, repeat=False)
        self.frame_gen = fbmc.simple_frame_generator_vcvc(sym_len=L, num_payload=1, inverse=0, num_overlap=4,
                                                   num_sync=7)  # the frame len includes the length of the overlap
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.frame_gen, self.snk)
        self.tb.run()
        # check data
        ref = (0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               1, 2, 3, 4, 5, 6, 7, 8,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0, 0, 0)
        ref = np.array(ref, dtype=np.complex)
        data = np.array(self.snk.data())
        print np.reshape(ref, (-1, L)).T
        print
        print np.reshape(data, (-1, L)).T
        self.assertComplexTuplesAlmostEqual(ref, data)

    def test_002_t(self):  # reverse
        print "test 2 - reverse"
        L = 8
        input_data = (0, 0, 0, 0, 0, 0, 0, 0,
                      1, 1, -1, -1, 1, 1, -1, -1,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      1, 2, 3, 4, 5, 6, 7, 8)
        self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
        self.frame_gen = fbmc.simple_frame_generator_vcvc(sym_len=L, num_payload=1, inverse=1, num_overlap=4, num_sync=3)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.frame_gen, self.snk)
        self.tb.run()
        # check data
        data = self.snk.data()
        ref = (1, 2, 3, 4, 5, 6, 7, 8)
        self.assertComplexTuplesAlmostEqual(ref, data)

    def test_003_t(self):  # forward
        print "test 3 - forward - many frames"
        L = 8
        overlap = 4
        preamble_len = 3
        sync_symbol_len = preamble_len + overlap
        payload_len = 1
        frame_len = sync_symbol_len + payload_len + overlap
        multiple = 20
        input_data = range(1, L * multiple + 1)
        self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
        self.frame_gen = fbmc.simple_frame_generator_vcvc(sym_len=L, num_payload=payload_len, inverse=0, num_overlap=overlap,
                                                   num_sync=sync_symbol_len)  # the frame len includes the length of the overlap
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.frame_gen, self.snk)
        self.tb.run()
        # check data
        data = self.snk.data()
        self.assertEqual(len(data), len(input_data) * frame_len)

    def test_004_t(self):  # reverse
        print "test 4 - reverse - many frames"
        L = 8
        K = 5
        num_frames = 10
        input_data = range(1, (K + 11) * L * num_frames + 1)
        self.src = blocks.vector_source_c(input_data, vlen=L, repeat=False)
        self.frame_gen = fbmc.simple_frame_generator_vcvc(sym_len=L, num_payload=K, inverse=1, num_overlap=4, num_sync=3)
        self.snk = blocks.vector_sink_c(vlen=L)
        self.tb.connect(self.src, self.frame_gen, self.snk)
        self.tb.run()
        # check data
        data = self.snk.data()
        self.assertEqual(len(data), L * K * num_frames)


if __name__ == '__main__':
    gr_unittest.run(qa_simple_frame_generator_vcvc)
