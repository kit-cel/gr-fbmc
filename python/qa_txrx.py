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
import numpy as np
import fbmc_test_functions as ft
import matplotlib.pyplot as plt


class qa_tx(gr_unittest.TestCase):
    def get_frame_len(self):
        return self.cfg.num_total_subcarriers() * self.cfg.num_sym_frame()

    def setUp(self):
        np.set_printoptions(2, linewidth=150)
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_frames(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = ft.get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = 8
        overlap = 4

        preamble = ft.get_preamble(total_subcarriers)
        payload = ft.get_payload(payload_symbols, used_subcarriers)
        payload = np.concatenate((payload, payload))

        src = blocks.vector_source_b(payload, repeat=False)
        framer = fbmc.frame_generator_bvc(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble)
        deframer = fbmc.deframer_vcb(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map)
        snk = blocks.vector_sink_b(1)
        self.tb.connect(src, framer, deframer, snk)
        self.tb.run()

        res = np.array(snk.data())
        print res
        print payload

        self.assertTupleEqual(tuple(payload), tuple(res))

    def test_002_modulation(self):
        total_subcarriers = 8
        overlap = 4
        taps = ft.generate_phydyas_filter(total_subcarriers, overlap)

        zvals = np.zeros(overlap * total_subcarriers, dtype=complex)
        rvals = np.arange(1, total_subcarriers + 1, dtype=complex)
        dummy_frame = np.concatenate((zvals, rvals))

        data = np.tile(dummy_frame, 4).flatten()


        src = blocks.vector_source_c(data, repeat=False, vlen=total_subcarriers)
        mod = fbmc.tx_sdft_vcc(taps, total_subcarriers)
        demod = fbmc.rx_sdft_cvc(taps, total_subcarriers)
        snk = blocks.vector_sink_c(total_subcarriers)

        self.tb.connect(src, mod, demod, snk)
        self.tb.run()

        res = np.array(snk.data())
        # print np.reshape(res, (-1, total_subcarriers)).T

        freq = np.fft.fft(np.reshape(res, (-1, total_subcarriers)))
        # print freq.T
        plt.plot(freq.flatten().real)
        for i in range(len(freq.flatten())):
            if (i + 1) % total_subcarriers == 0:
                plt.axvline(i)
        plt.plot(5 * data.real)

        plt.grid()
        plt.show()

    def test_003_small_frame_mod(self):
        total_subcarriers = 8
        used_subcarriers = 4
        channel_map = np.array((0, 0, 1, 1, 1, 1, 0, 0)) #ft.get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = 8
        overlap = 4
        taps = ft.generate_phydyas_filter(total_subcarriers, overlap)

        preamble = ft.get_preamble(total_subcarriers)
        print np.reshape(preamble, (-1, total_subcarriers)).T
        payload = ft.get_payload(payload_symbols, used_subcarriers)
        # payload = np.ones(payload_symbols * used_subcarriers // 2, dtype=int) # np.array((1, ) * payload_symbols * used_subcarriers // 2)
        payload = np.tile(payload, 5)


        src = blocks.vector_source_b(payload, repeat=False)
        framer = fbmc.frame_generator_bvc(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble)
        snk_frame = blocks.vector_sink_c(total_subcarriers)  # a debug output

        mod = fbmc.tx_sdft_vcc(taps, total_subcarriers)
        demod = fbmc.rx_sdft_cvc(taps, total_subcarriers)
        skipper = blocks.skiphead(8 * total_subcarriers, 4)

        snk_rx = blocks.vector_sink_c(total_subcarriers)
        deframer = fbmc.deframer_vcb(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map)
        snk = blocks.vector_sink_b(1)
        self.tb.connect(src, framer, mod, demod, skipper, deframer, snk)
        self.tb.connect(framer, snk_frame)
        self.tb.connect(skipper, snk_rx)
        self.tb.run()

        res = np.array(snk.data())
        print "len(res) = ", len(res), ", len(payload) = ", len(payload)
        print "ref: ", payload
        print "res: ", res

        moddata = np.array(snk_frame.data())
        print "len(moddata) = ", len(moddata)
        rxdata = np.array(snk_rx.data())
        print "len(rxdata) = ", len(rxdata)

        # plt.plot(rxdata.real * 0.03)
        # for i in range(len(moddata)):
        #     if (i + 1) % total_subcarriers == 0:
        #         plt.axvline(i)
        # plt.plot(moddata.real)
        # plt.grid()
        # plt.show()

        self.assertTupleEqual(tuple(payload), tuple(res))

    def test_004_config_frame_mod(self):
        cfg = fbmc.fbmc_config(num_used_subcarriers=20, num_payload_sym=16, num_overlap_sym=4, modulation="QPSK",
                                    preamble="IAM", samp_rate=250000)
        total_subcarriers = cfg.num_total_subcarriers()  # 8
        used_subcarriers = cfg.num_used_subcarriers()  # 4
        channel_map = cfg.channel_map()  # ft.get_channel_map(used_subcarriers, total_subcarriers)
        payload_symbols = cfg.num_payload_sym()  # 8
        overlap = cfg.num_overlap_sym()  # 4
        taps = cfg.phydyas_impulse_taps(cfg.num_total_subcarriers(), cfg.num_overlap_sym())  # ft.generate_phydyas_filter(total_subcarriers, overlap)

        preamble = ft.get_preamble(total_subcarriers)
        payload = ft.get_payload(payload_symbols, used_subcarriers)
        payload = np.concatenate((payload, payload))

        src = blocks.vector_source_b(payload, repeat=False)
        framer = fbmc.frame_generator_bvc(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble)
        snk_frame = blocks.vector_sink_c(total_subcarriers)  # a debug output

        # skiphead 2 will force correct reception.
        mod = fbmc.tx_sdft_vcc(taps, total_subcarriers)
        demod = fbmc.rx_sdft_cvc(taps, total_subcarriers)
        skipper = blocks.skiphead(8 * total_subcarriers, 4)

        snk_rx = blocks.vector_sink_c(total_subcarriers)
        deframer = fbmc.deframer_vcb(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map)
        snk = blocks.vector_sink_b(1)
        self.tb.connect(src, framer, mod, demod, skipper, deframer, snk)
        self.tb.connect(framer, snk_frame)
        self.tb.connect(skipper, snk_rx)
        self.tb.run()

        res = np.array(snk.data())
        print "len(res) = ", len(res), ", len(payload) = ", len(payload)
        print res
        print payload

        moddata = np.array(snk_frame.data())
        print "len(moddata) = ", len(moddata)
        rxdata = np.array(snk_rx.data())
        print "len(rxdata) = ", len(rxdata)

        plt.plot(rxdata.real * 0.03)
        for i in range(len(moddata)):
            if (i + 1) % total_subcarriers == 0:
                plt.axvline(i)
        plt.plot(moddata.real)
        plt.grid()
        plt.show()

        self.assertTupleEqual(tuple(payload), tuple(res))

if __name__ == '__main__':
    gr_unittest.run(qa_tx)
