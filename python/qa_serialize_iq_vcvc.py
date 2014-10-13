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


class qa_serialize_iq_vcvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        L = 8
        multiple = 10
        d = self.generate_vector(L)
        res = self.serialize_vector(d)
        in_data = []
        out_data = []
        for i in range(multiple):
            in_data.extend(d)
            out_data.extend(res)

        d = in_data
        res = out_data

        # set up fg
        self.src = blocks.vector_source_c(d)
        self.ser = fbmc.serialize_iq_vcvc(L)
        self.snk = blocks.vector_sink_c()
        self.tb.connect(self.src, self.ser, self.snk)
        self.tb.run()

        data = self.snk.data()
        self.assertComplexTuplesAlmostEqual(data, res)

    def test_002_t(self):
        L = 64
        n = L * 1000
        input_data = range(n)
        self.src = blocks.vector_source_c(input_data)
        self.ser = fbmc.serialize_iq_vcvc(L)
        self.snk = blocks.vector_sink_c()
        self.tb.connect(self.src, self.ser, self.snk)
        # set up fg
        self.tb.run()
        data = self.snk.data()
        # check data
        self.assertTrue(len(data), n * 2)

    def generate_vector(self, L):
        d = [complex(i, i + L) for i in range(0, L)]
        return d

    def serialize_vector(self, d):
        L = int(len(d))
        di = [complex(d[i].real, 0) for i in range(L)]
        dq = [complex(d[i].imag, 0) for i in range(L)]
        di.extend(dq)
        return di


if __name__ == '__main__':
    gr_unittest.run(qa_serialize_iq_vcvc)
