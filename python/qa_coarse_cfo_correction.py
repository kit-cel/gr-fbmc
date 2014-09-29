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

class qa_coarse_cfo_correction (gr_unittest.TestCase):

    def setUp (self):
    	self.cfg = fbmc.fbmc_config(num_used_subcarriers=8, num_payload_sym=18, num_overlap_sym=4, modulation="QPSK", preamble="IAM")
        self.tb = gr.top_block ()
        self.cfo = fbmc.coarse_cfo_correction(self.cfg.channel_map())

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        self.tb.run ()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_coarse_cfo_correction, "qa_coarse_cfo_correction.xml")
