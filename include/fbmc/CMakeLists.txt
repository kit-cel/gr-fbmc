# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Install public header files
########################################################################
install(FILES
    ../../lib/fbmc_config.h
    api.h
    apply_betas_vcvc.h
    serial_to_parallel_cvc.h
    serialize_iq_vcvc.h
    polyphase_filterbank_vcvc.h
    output_commutator_vcc.h
    simple_frame_generator_vcvc.h
    input_commutator_cvc.h
    combine_iq_vcvc.h
    parallel_to_serial_vcc.h
    symbols_to_bits_cb.h
    preamble_insertion_vcvc.h
    frame_sync_cc.h
    coarse_cfo_correction.h
    rx_polyphase_cvc.h
    rx_polyphase_kernel.h
    rx_sdft_cvc.h
    rx_sdft_kernel.h
    tx_sdft_vcc.h
    tx_sdft_kernel.h
    rx_domain_cvc.h
    rx_domain_kernel.h
    smt_kernel.h
    frame_generator_bvc.h
    deframer_vcb.h
    multichannel_frame_generator_bvc.h
    multichannel_deframer_vcb.h
    multichannel_frame_sync_cc.h
    time_freq_sync_cc.h
    phase_sync_cc.h
    subchannel_frame_generator_bvc.h
    rx_freq_despread_cvc.h
    subchannel_deframer_vcb.h DESTINATION include/fbmc
)
