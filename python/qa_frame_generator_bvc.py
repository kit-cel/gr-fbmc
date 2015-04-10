#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2015 <+YOU OR YOUR COMPANY+>.
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
import numpy as np


class qa_frame_generator_bvc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = [1, 1, 0, 0, 1, 1, 0, 0]
        payload_symbols = 8
        overlap = 4
        preamble = self._get_preamble(total_subcarriers)

        # set up fg
        self.tb.run()
        # check data

    def _get_preamble(self, total_subcarriers):
        preamble = np.ones(total_subcarriers) * 2
        print preamble
        preamble = np.concatenate((preamble, np.zeros(total_subcarriers)))
        preamble = np.concatenate((preamble, np.ones(total_subcarriers) * 4))
        preamble = np.concatenate((preamble, np.ones(total_subcarriers)))
        return preamble

if __name__ == '__main__':
    gr_unittest.run(qa_frame_generator_bvc)
