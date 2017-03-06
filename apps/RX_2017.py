#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Rx 2017
# Generated: Mon Mar  6 19:07:07 2017
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt4 import Qt
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
from time_sync import time_sync  # grc-generated hier_block
import classifier
import fbmc
import numpy as np
import time


class RX_2017(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Rx 2017")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Rx 2017")
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "RX_2017")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.subchan_map = subchan_map = np.concatenate(([0,]*6, [1,]*116, [0,]*6))
        self.samp_rate = samp_rate = int(1e7)
        self.packetlen_base = packetlen_base = 256 * 8 * 3
        self.cfg = cfg = fbmc.fbmc_config(channel_map=(subchan_map), num_payload_bits=packetlen_base, num_overlap_sym=4, samp_rate=int(samp_rate)/4)
        self.taps = taps = [0.003524782368913293, 0.002520402194932103, -3.532667373554307e-17, -0.0025783423334360123, -0.0036887258756905794, -0.0026390093844383955, -3.7046301165340785e-18, 0.0027025998570024967, 0.003868663916364312, 0.0027693307492882013, -9.712380039780164e-18, -0.0028394402470439672, -0.004067056812345982, -0.0029131921473890543, 2.454170927498071e-17, 0.0029908770229667425, 0.004286897834390402, 0.0030728192068636417, -9.712380039780164e-18, -0.0031593774911016226, -0.004531863145530224, -0.0032509532757103443, -6.861573196148834e-18, 0.0033479968551546335, 0.004806521814316511, 0.003451012307778001, -9.712380039780164e-18, -0.0035605679731816053, -0.005116619635373354, -0.0036773078609257936, 2.849619674016194e-17, 0.003801962360739708, 0.005469490308314562, 0.003935364540666342, -9.712380039780164e-18, -0.004078468773514032, -0.0058746375143527985, -0.004232373554259539, -1.196125168755972e-17, 0.004398348741233349, 0.006344608962535858, 0.004577873274683952, -9.712380039780164e-18, -0.004772676154971123, -0.0068963137455284595, -0.004984795115888119, 3.532667373554307e-17, 0.005216646008193493, 0.00755310570821166, 0.005471116863191128, -9.712380039780164e-18, -0.0057516866363584995, -0.00834816973656416, -0.00606258912011981, 9.712380039780164e-18, 0.006409022491425276, 0.009330307133495808, 0.006797448266297579, -9.712380039780164e-18, -0.007235993165522814, -0.010574348270893097, -0.007735027000308037, 9.712380039780164e-18, 0.008307991549372673, 0.012201170437037945, 0.008972631767392159, -9.712380039780164e-18, -0.00975286029279232, -0.014419564977288246, -0.010681704618036747, 9.712380039780164e-18, 0.011806094087660313, 0.017623912543058395, 0.013195047155022621, -9.712380039780164e-18, -0.01495438627898693, -0.022659316658973694, -0.017255060374736786, 9.712380039780164e-18, 0.020392345264554024, 0.03172304481267929, 0.02492397651076317, -9.712380039780164e-18, -0.03204511106014252, -0.052871737629175186, -0.04486315697431564, 9.712380039780164e-18, 0.0747719332575798, 0.15861520171165466, 0.22431577742099762, 0.24915219843387604, 0.22431577742099762, 0.15861520171165466, 0.0747719332575798, 9.712380039780164e-18, -0.04486315697431564, -0.052871737629175186, -0.03204511106014252, -9.712380039780164e-18, 0.02492397651076317, 0.03172304481267929, 0.020392345264554024, 9.712380039780164e-18, -0.017255060374736786, -0.022659316658973694, -0.01495438627898693, -9.712380039780164e-18, 0.013195047155022621, 0.017623912543058395, 0.011806094087660313, 9.712380039780164e-18, -0.010681704618036747, -0.014419564977288246, -0.00975286029279232, -9.712380039780164e-18, 0.008972631767392159, 0.012201170437037945, 0.008307991549372673, 9.712380039780164e-18, -0.007735027000308037, -0.010574348270893097, -0.007235993165522814, -9.712380039780164e-18, 0.006797448266297579, 0.009330307133495808, 0.006409022491425276, 9.712380039780164e-18, -0.00606258912011981, -0.00834816973656416, -0.0057516866363584995, -9.712380039780164e-18, 0.005471116863191128, 0.00755310570821166, 0.005216646008193493, 3.532667373554307e-17, -0.004984795115888119, -0.0068963137455284595, -0.004772676154971123, -9.712380039780164e-18, 0.004577873274683952, 0.006344608962535858, 0.004398348741233349, -1.196125168755972e-17, -0.004232373554259539, -0.0058746375143527985, -0.004078468773514032, -9.712380039780164e-18, 0.003935364540666342, 0.005469490308314562, 0.003801962360739708, 2.849619674016194e-17, -0.0036773078609257936, -0.005116619635373354, -0.0035605679731816053, -9.712380039780164e-18, 0.003451012307778001, 0.004806521814316511, 0.0033479968551546335, -6.861573196148834e-18, -0.0032509532757103443, -0.004531863145530224, -0.0031593774911016226, -9.712380039780164e-18, 0.0030728192068636417, 0.004286897834390402, 0.0029908770229667425, 2.454170927498071e-17, -0.0029131921473890543, -0.004067056812345982, -0.0028394402470439672, -9.712380039780164e-18, 0.0027693307492882013, 0.003868663916364312, 0.0027025998570024967, -3.7046301165340785e-18, -0.0026390093844383955, -0.0036887258756905794, -0.0025783423334360123, -3.532667373554307e-17, 0.002520402194932103]
        self.phydyas_taps_time = phydyas_taps_time = np.array(cfg.phydyas_impulse_taps(cfg.num_total_subcarriers(), cfg.num_overlap_sym()))
        self.nguard_bins = nguard_bins = 8
        self.nchan = nchan = 4
        self.sync = sync = fbmc.sync_config(taps=(phydyas_taps_time[1:]/np.sqrt(phydyas_taps_time.dot(phydyas_taps_time))), N=cfg.num_total_subcarriers(), overlap=4, L=cfg.num_total_subcarriers()-1, pilot_A=1.0, pilot_timestep=4, pilot_carriers=(range(8, 118, 12) + [119]), subbands=nchan, bits=packetlen_base, pos=4, u=1, q=4, A=1.0 , fft_len=2**13, guard=nguard_bins, order=8)
        self.su_frame_len_low_rate = su_frame_len_low_rate = sync.get_frame_samps(True)
        self.rx_gain = rx_gain = .9
        self.fine_frequency_correction = fine_frequency_correction = 0
        self.delay_offset = delay_offset = 1120
        self.corr_len = corr_len = len(sync.get_fir_sequences()[0])
        self.const = const = 1000
        self.cfreq = cfreq = uhd.tune_request(3.195e9, samp_rate)

        ##################################################
        # Blocks
        ##################################################
        self._rx_gain_range = Range(0, 1, .05, .9, 200)
        self._rx_gain_win = RangeWidget(self._rx_gain_range, self.set_rx_gain, 'RX gain (normalized)', "counter_slider", float)
        self.top_grid_layout.addWidget(self._rx_gain_win, 0, 0, 1, 2)
        self._delay_offset_range = Range(0, 2000, 1, 1120, 200)
        self._delay_offset_win = RangeWidget(self._delay_offset_range, self.set_delay_offset, "delay_offset", "counter_slider", int)
        self.top_layout.addWidget(self._delay_offset_win)
        self._const_range = Range(0, 1000, .1, 1000, 200)
        self._const_win = RangeWidget(self._const_range, self.set_const, "const", "counter_slider", float)
        self.top_layout.addWidget(self._const_win)
        self.uhd_usrp_source_0 = uhd.usrp_source(
        	",".join(("name=Chasmine", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0.set_clock_source('gpsdo', 0)
        self.uhd_usrp_source_0.set_time_source('gpsdo', 0)
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0.set_center_freq(cfreq, 0)
        self.uhd_usrp_source_0.set_normalized_gain(rx_gain, 0)
        self.uhd_usrp_source_0.set_antenna('RX2', 0)
        self.time_sync_0 = time_sync(
            constant=const,
            fir_sequences=sync.get_fir_sequences(),
            frame_len=sync.get_frame_samps(True) * nchan,
            peak_delay=delay_offset,
        )
        self._fine_frequency_correction_range = Range(-10000, 10000, 10, 0, 200)
        self._fine_frequency_correction_win = RangeWidget(self._fine_frequency_correction_range, self.set_fine_frequency_correction, 'Fine frequency correction', "counter_slider", float)
        self.top_layout.addWidget(self._fine_frequency_correction_win)
        self.fbmc_subchannel_deframer_vcb_0 = fbmc.subchannel_deframer_vcb(cfg.num_total_subcarriers(), nchan, nguard_bins, 0.9, (sync.get_preamble_symbols()), sync.get_syms_frame(), cfg.num_payload_bits(), (sync.get_pilot_carriers()), sync.get_pilot_timestep(), sync.get_bps())
        self.fbmc_sliding_fft_cvc_0 = fbmc.sliding_fft_cvc(cfg.num_total_subcarriers(), cfg.num_overlap_sym(), nchan, sync.get_syms_frame())
        self.fbmc_channel_estimator_vcvc_0 = fbmc.channel_estimator_vcvc(sync.get_syms_frame(), cfg.num_total_subcarriers(), cfg.num_overlap_sym(), nchan, (np.array(cfg.phydyas_frequency_taps(cfg.num_overlap_sym()))), sync.get_pilot_amplitude(), sync.get_pilot_timestep(), (sync.get_pilot_carriers()))
        self.fbmc_channel_equalizer_vcvc_0 = fbmc.channel_equalizer_vcvc(sync.get_syms_frame(), cfg.num_overlap_sym(), nchan, sync.get_pilot_timestep(), (sync.get_pilot_carriers()), cfg.num_total_subcarriers(), (cfg.phydyas_frequency_taps(cfg.num_overlap_sym())), sync.get_pilot_amplitude())
        self.classifier_packet_sink_0 = classifier.packet_sink("packet_len","127.0.0.1",5002,64,0)
        self.blocks_unpacked_to_packed_xx_0 = blocks.unpacked_to_packed_bb(sync.get_bps(), gr.GR_LSB_FIRST)
        self.blocks_stream_to_tagged_stream_0 = blocks.stream_to_tagged_stream(gr.sizeof_char, 1, 64, "packet_len")
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_float*1)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_stream_to_tagged_stream_0, 0), (self.classifier_packet_sink_0, 0))    
        self.connect((self.blocks_unpacked_to_packed_xx_0, 0), (self.blocks_stream_to_tagged_stream_0, 0))    
        self.connect((self.fbmc_channel_equalizer_vcvc_0, 0), (self.fbmc_subchannel_deframer_vcb_0, 0))    
        self.connect((self.fbmc_channel_estimator_vcvc_0, 1), (self.fbmc_channel_equalizer_vcvc_0, 1))    
        self.connect((self.fbmc_channel_estimator_vcvc_0, 0), (self.fbmc_channel_equalizer_vcvc_0, 0))    
        self.connect((self.fbmc_sliding_fft_cvc_0, 0), (self.fbmc_channel_estimator_vcvc_0, 0))    
        self.connect((self.fbmc_subchannel_deframer_vcb_0, 0), (self.blocks_unpacked_to_packed_xx_0, 0))    
        self.connect((self.time_sync_0, 1), (self.blocks_null_sink_0, 0))    
        self.connect((self.time_sync_0, 0), (self.fbmc_sliding_fft_cvc_0, 0))    
        self.connect((self.uhd_usrp_source_0, 0), (self.time_sync_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "RX_2017")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_subchan_map(self):
        return self.subchan_map

    def set_subchan_map(self, subchan_map):
        self.subchan_map = subchan_map

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_cfreq(uhd.tune_request(3.195e9, self.samp_rate))
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)

    def get_packetlen_base(self):
        return self.packetlen_base

    def set_packetlen_base(self, packetlen_base):
        self.packetlen_base = packetlen_base

    def get_cfg(self):
        return self.cfg

    def set_cfg(self, cfg):
        self.cfg = cfg

    def get_taps(self):
        return self.taps

    def set_taps(self, taps):
        self.taps = taps

    def get_phydyas_taps_time(self):
        return self.phydyas_taps_time

    def set_phydyas_taps_time(self, phydyas_taps_time):
        self.phydyas_taps_time = phydyas_taps_time

    def get_nguard_bins(self):
        return self.nguard_bins

    def set_nguard_bins(self, nguard_bins):
        self.nguard_bins = nguard_bins

    def get_nchan(self):
        return self.nchan

    def set_nchan(self, nchan):
        self.nchan = nchan
        self.time_sync_0.set_frame_len(sync.get_frame_samps(True) * self.nchan)

    def get_sync(self):
        return self.sync

    def set_sync(self, sync):
        self.sync = sync

    def get_su_frame_len_low_rate(self):
        return self.su_frame_len_low_rate

    def set_su_frame_len_low_rate(self, su_frame_len_low_rate):
        self.su_frame_len_low_rate = su_frame_len_low_rate

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_0.set_normalized_gain(self.rx_gain, 0)
        	

    def get_fine_frequency_correction(self):
        return self.fine_frequency_correction

    def set_fine_frequency_correction(self, fine_frequency_correction):
        self.fine_frequency_correction = fine_frequency_correction

    def get_delay_offset(self):
        return self.delay_offset

    def set_delay_offset(self, delay_offset):
        self.delay_offset = delay_offset
        self.time_sync_0.set_peak_delay(self.delay_offset)

    def get_corr_len(self):
        return self.corr_len

    def set_corr_len(self, corr_len):
        self.corr_len = corr_len

    def get_const(self):
        return self.const

    def set_const(self, const):
        self.const = const
        self.time_sync_0.set_constant(self.const)

    def get_cfreq(self):
        return self.cfreq

    def set_cfreq(self, cfreq):
        self.cfreq = cfreq
        self.uhd_usrp_source_0.set_center_freq(self.cfreq, 0)


def main(top_block_cls=RX_2017, options=None):
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print "Error: failed to enable real-time scheduling."

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
