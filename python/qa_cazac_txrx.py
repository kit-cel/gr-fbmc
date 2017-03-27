#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: qa_tx_rx
# Generated: Thu Mar  9 17:09:31 2017
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr, gr_unittest
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import fbmc
import numpy
import numpy as np


class qa_tx_rx(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()
        ##################################################
        # Variables
        ##################################################
        self.tb.subchan_map = subchan_map = np.concatenate(([0, ] * 2, [1, ] * 124, [0, ] * 2))
        self.tb.samp_rate = samp_rate = 10e6
        self.tb.cfg = cfg = fbmc.fbmc_config(channel_map=(subchan_map), num_payload_bits=256 * 8 * 3, num_overlap_sym=4,
                                          samp_rate=int(samp_rate) / 4)
        self.tb.tx_att_db = tx_att_db = 2
        self.tb.phydyas_taps_time = phydyas_taps_time = np.array(
            cfg.phydyas_impulse_taps(cfg.num_total_subcarriers(), cfg.num_overlap_sym()))
        self.tb.bands = bands = 4
        self.tb.sync = sync = fbmc.sync_config(
            N=cfg.num_total_subcarriers(), overlap=4, L=127, pilot_A=1.0, pilot_timestep=4,
            pilot_carriers=(range(8, 118, 12) + [119]), subbands=bands, bits=256 * 8 * 3, pos=4, u=1, q=4,
            A=1.0 * 10 ** (-tx_att_db / 20), fft_len=2 ** 13, guard=8, order=8)
        self.tb.packetlen_base = packetlen_base = 32 * 8
        self.tb.npackets = npackets = 1
        self.tb.fir = fir = np.conj(np.fft.ifft(
            np.fft.ifftshift(np.hstack((np.fft.fftshift(np.fft.fft(sync.get_zadoff_chu(64))), np.zeros(3 * 64))))))
        self.tb.channel = channel = [h * np.exp(1j * 2 * np.pi * np.random.random()) for h in
                                  [0.98304252, 0.01179651, 0.00516097]]

        ##################################################
        # Blocks
        ##################################################
        self.tb.interp_fir_filter_xxx_0 = filter.interp_fir_filter_ccc(bands, (
        [0.003524782368913293, 0.002520402194932103, -3.532667373554307e-17, -0.0025783423334360123,
         -0.0036887258756905794, -0.0026390093844383955, -3.7046301165340785e-18, 0.0027025998570024967,
         0.003868663916364312, 0.0027693307492882013, -9.712380039780164e-18, -0.0028394402470439672,
         -0.004067056812345982, -0.0029131921473890543, 2.454170927498071e-17, 0.0029908770229667425,
         0.004286897834390402, 0.0030728192068636417, -9.712380039780164e-18, -0.0031593774911016226,
         -0.004531863145530224, -0.0032509532757103443, -6.861573196148834e-18, 0.0033479968551546335,
         0.004806521814316511, 0.003451012307778001, -9.712380039780164e-18, -0.0035605679731816053,
         -0.005116619635373354, -0.0036773078609257936, 2.849619674016194e-17, 0.003801962360739708,
         0.005469490308314562, 0.003935364540666342, -9.712380039780164e-18, -0.004078468773514032,
         -0.0058746375143527985, -0.004232373554259539, -1.196125168755972e-17, 0.004398348741233349,
         0.006344608962535858, 0.004577873274683952, -9.712380039780164e-18, -0.004772676154971123,
         -0.0068963137455284595, -0.004984795115888119, 3.532667373554307e-17, 0.005216646008193493,
         0.00755310570821166, 0.005471116863191128, -9.712380039780164e-18, -0.0057516866363584995,
         -0.00834816973656416, -0.00606258912011981, 9.712380039780164e-18, 0.006409022491425276, 0.009330307133495808,
         0.006797448266297579, -9.712380039780164e-18, -0.007235993165522814, -0.010574348270893097,
         -0.007735027000308037, 9.712380039780164e-18, 0.008307991549372673, 0.012201170437037945, 0.008972631767392159,
         -9.712380039780164e-18, -0.00975286029279232, -0.014419564977288246, -0.010681704618036747,
         9.712380039780164e-18, 0.011806094087660313, 0.017623912543058395, 0.013195047155022621,
         -9.712380039780164e-18, -0.01495438627898693, -0.022659316658973694, -0.017255060374736786,
         9.712380039780164e-18, 0.020392345264554024, 0.03172304481267929, 0.02492397651076317, -9.712380039780164e-18,
         -0.03204511106014252, -0.052871737629175186, -0.04486315697431564, 9.712380039780164e-18, 0.0747719332575798,
         0.15861520171165466, 0.22431577742099762, 0.24915219843387604, 0.22431577742099762, 0.15861520171165466,
         0.0747719332575798, 9.712380039780164e-18, -0.04486315697431564, -0.052871737629175186, -0.03204511106014252,
         -9.712380039780164e-18, 0.02492397651076317, 0.03172304481267929, 0.020392345264554024, 9.712380039780164e-18,
         -0.017255060374736786, -0.022659316658973694, -0.01495438627898693, -9.712380039780164e-18,
         0.013195047155022621, 0.017623912543058395, 0.011806094087660313, 9.712380039780164e-18, -0.010681704618036747,
         -0.014419564977288246, -0.00975286029279232, -9.712380039780164e-18, 0.008972631767392159,
         0.012201170437037945, 0.008307991549372673, 9.712380039780164e-18, -0.007735027000308037,
         -0.010574348270893097, -0.007235993165522814, -9.712380039780164e-18, 0.006797448266297579,
         0.009330307133495808, 0.006409022491425276, 9.712380039780164e-18, -0.00606258912011981, -0.00834816973656416,
         -0.0057516866363584995, -9.712380039780164e-18, 0.005471116863191128, 0.00755310570821166,
         0.005216646008193493, 3.532667373554307e-17, -0.004984795115888119, -0.0068963137455284595,
         -0.004772676154971123, -9.712380039780164e-18, 0.004577873274683952, 0.006344608962535858,
         0.004398348741233349, -1.196125168755972e-17, -0.004232373554259539, -0.0058746375143527985,
         -0.004078468773514032, -9.712380039780164e-18, 0.003935364540666342, 0.005469490308314562,
         0.003801962360739708, 2.849619674016194e-17, -0.0036773078609257936, -0.005116619635373354,
         -0.0035605679731816053, -9.712380039780164e-18, 0.003451012307778001, 0.004806521814316511,
         0.0033479968551546335, -6.861573196148834e-18, -0.0032509532757103443, -0.004531863145530224,
         -0.0031593774911016226, -9.712380039780164e-18, 0.0030728192068636417, 0.004286897834390402,
         0.0029908770229667425, 2.454170927498071e-17, -0.0029131921473890543, -0.004067056812345982,
         -0.0028394402470439672, -9.712380039780164e-18, 0.0027693307492882013, 0.003868663916364312,
         0.0027025998570024967, -3.7046301165340785e-18, -0.0026390093844383955, -0.0036887258756905794,
         -0.0025783423334360123, -3.532667373554307e-17, 0.002520402194932103]))
        self.tb.interp_fir_filter_xxx_0.declare_sample_delay(0)
        self.tb.fir_filter_xxx_0 = filter.fir_filter_ccc(1, (channel))
        self.tb.fir_filter_xxx_0.declare_sample_delay(0)
        self.tb.fbmc_tx_sdft_vcc_0 = fbmc.tx_sdft_vcc(
            (phydyas_taps_time / np.sqrt(phydyas_taps_time.dot(phydyas_taps_time)) / 2), cfg.num_total_subcarriers())
        self.tb.fbmc_tx_dummy_mixer_cc_0 = fbmc.tx_dummy_mixer_cc(bands, samp_rate / bands, cfg.num_payload_bits() / (
        cfg.num_total_subcarriers() - len(sync.get_pilot_carriers()) / sync.get_pilot_timestep()) + 1,
                                                               cfg.num_total_subcarriers(), samp_rate)
        self.tb.fbmc_subchannel_frame_generator_bvc_0 = fbmc.subchannel_frame_generator_bvc(cfg.num_total_subcarriers(), 8,
                                                                                         cfg.num_payload_bits(),
                                                                                         cfg.num_overlap_sym(),
                                                                                         (sync.get_preamble_symbols()),
                                                                                         sync.get_pilot_amplitude(),
                                                                                         sync.get_pilot_timestep(),
                                                                                         (sync.get_pilot_carriers()),
                                                                                         sync.get_syms_frame(), True,
                                                                                         sync.get_bps())
        self.tb.fbmc_subchannel_deframer_vcb_0 = fbmc.subchannel_deframer_vcb(cfg.num_total_subcarriers(), bands, 8, 0.9,
                                                                           (sync.get_preamble_symbols()),
                                                                           sync.get_syms_frame(),
                                                                           cfg.num_payload_bits(),
                                                                           (sync.get_pilot_carriers()),
                                                                           sync.get_pilot_timestep(), sync.get_bps())
        self.tb.fbmc_sliding_fft_cvc_0 = fbmc.sliding_fft_cvc(cfg.num_total_subcarriers(), cfg.num_overlap_sym(), bands,
                                                           sync.get_syms_frame())
        self.tb.fbmc_channel_estimator_vcvc_0 = fbmc.channel_estimator_vcvc(sync.get_syms_frame(),
                                                                         cfg.num_total_subcarriers(),
                                                                         cfg.num_overlap_sym(), bands, (np.array(
                cfg.phydyas_frequency_taps(cfg.num_overlap_sym()))), sync.get_pilot_amplitude(),
                                                                         sync.get_pilot_timestep(),
                                                                         (sync.get_pilot_carriers()))
        self.tb.fbmc_channel_equalizer_vcvc_0 = fbmc.channel_equalizer_vcvc(sync.get_syms_frame(), cfg.num_overlap_sym(),
                                                                         bands, sync.get_pilot_timestep(),
                                                                         (sync.get_pilot_carriers()),
                                                                         cfg.num_total_subcarriers(), (
                                                                         cfg.phydyas_frequency_taps(
                                                                             cfg.num_overlap_sym())),
                                                                         sync.get_pilot_amplitude())
        self.tb.blocks_vector_sink_x_0_0 = blocks.vector_sink_b(10000)
        self.tb.blocks_vector_sink_x_0 = blocks.vector_sink_b(10000)
        self.tb.blocks_unpacked_to_packed_xx_0 = blocks.unpacked_to_packed_bb(sync.get_bps(), gr.GR_LSB_FIRST)
        self.tb.blocks_stream_to_vector_0_0 = blocks.stream_to_vector(gr.sizeof_char * 1, 10000)
        self.tb.blocks_stream_to_vector_0 = blocks.stream_to_vector(gr.sizeof_char * 1, 10000)
        self.tb.blocks_skiphead_0 = blocks.skiphead(gr.sizeof_gr_complex * 1, 90)
        self.tb.blocks_packed_to_unpacked_xx_0 = blocks.packed_to_unpacked_bb(sync.get_bps(), gr.GR_LSB_FIRST)
        self.tb.analog_random_source_x_0 = blocks.vector_source_b(map(int, numpy.random.randint(0, 255, 10000)), False)

        ##################################################
        # Connections
        ##################################################
        self.tb.connect((self.tb.analog_random_source_x_0, 0), (self.tb.blocks_packed_to_unpacked_xx_0, 0))
        self.tb.connect((self.tb.analog_random_source_x_0, 0), (self.tb.blocks_stream_to_vector_0_0, 0))
        self.tb.connect((self.tb.blocks_packed_to_unpacked_xx_0, 0), (self.tb.fbmc_subchannel_frame_generator_bvc_0, 0))
        self.tb.connect((self.tb.blocks_skiphead_0, 0), (self.tb.fbmc_tx_dummy_mixer_cc_0, 0))
        self.tb.connect((self.tb.blocks_stream_to_vector_0, 0), (self.tb.blocks_vector_sink_x_0, 0))
        self.tb.connect((self.tb.blocks_stream_to_vector_0_0, 0), (self.tb.blocks_vector_sink_x_0_0, 0))
        self.tb.connect((self.tb.blocks_unpacked_to_packed_xx_0, 0), (self.tb.blocks_stream_to_vector_0, 0))
        self.tb.connect((self.tb.fbmc_channel_equalizer_vcvc_0, 0), (self.tb.fbmc_subchannel_deframer_vcb_0, 0))
        self.tb.connect((self.tb.fbmc_channel_estimator_vcvc_0, 0), (self.tb.fbmc_channel_equalizer_vcvc_0, 1))
        self.tb.connect((self.tb.fbmc_sliding_fft_cvc_0, 0), (self.tb.fbmc_channel_equalizer_vcvc_0, 0))
        self.tb.connect((self.tb.fbmc_sliding_fft_cvc_0, 0), (self.tb.fbmc_channel_estimator_vcvc_0, 0))
        self.tb.connect((self.tb.fbmc_subchannel_deframer_vcb_0, 0), (self.tb.blocks_unpacked_to_packed_xx_0, 0))
        self.tb.connect((self.tb.fbmc_subchannel_frame_generator_bvc_0, 0), (self.tb.fbmc_tx_sdft_vcc_0, 0))
        self.tb.connect((self.tb.fbmc_tx_dummy_mixer_cc_0, 0), (self.tb.fir_filter_xxx_0, 0))
        self.tb.connect((self.tb.fbmc_tx_sdft_vcc_0, 0), (self.tb.interp_fir_filter_xxx_0, 0))
        self.tb.connect((self.tb.fir_filter_xxx_0, 0), (self.tb.fbmc_sliding_fft_cvc_0, 0))
        self.tb.connect((self.tb.interp_fir_filter_xxx_0, 0), (self.tb.blocks_skiphead_0, 0))

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        # set up fg
        self.tb.run()
        # check data
        sent = self.tb.blocks_vector_sink_x_0_0.data()
        received = self.tb.blocks_vector_sink_x_0.data()
        length = len(received)

        self.assertTupleEqual(received, sent[:length])

if __name__ == '__main__':
    gr_unittest.run(qa_tx_rx, "qa_tx_rx.xml")
