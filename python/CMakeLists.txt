# Copyright 2011 Free Software Foundation, Inc.
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
# Include python install macros
########################################################################
include(GrPython)
if(NOT PYTHONINTERP_FOUND)
    return()
endif()

########################################################################
# Install python sources
########################################################################
GR_PYTHON_INSTALL(
    FILES
    __init__.py
    preamble_config.py
    DESTINATION ${GR_PYTHON_DIR}/fbmc
)

########################################################################
# Handle the unit tests
########################################################################
include(GrTest)

set(GR_TEST_TARGET_DEPS gnuradio-fbmc)
set(GR_TEST_PYTHON_DIRS ${CMAKE_BINARY_DIR}/swig)
GR_ADD_TEST(qa_apply_betas_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_apply_betas_vcvc.py)
GR_ADD_TEST(qa_serial_to_parallel_cvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_serial_to_parallel_cvc.py)
GR_ADD_TEST(qa_serialize_iq_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_serialize_iq_vcvc.py)
GR_ADD_TEST(qa_polyphase_filterbank_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_polyphase_filterbank_vcvc.py)
GR_ADD_TEST(qa_output_commutator_vcc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_output_commutator_vcc.py)
GR_ADD_TEST(qa_txrx ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_txrx.py)
GR_ADD_TEST(qa_simple_frame_generator_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_simple_frame_generator_vcvc.py)
GR_ADD_TEST(qa_input_commutator_cvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_input_commutator_cvc.py)
GR_ADD_TEST(qa_combine_iq_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_combine_iq_vcvc.py)
GR_ADD_TEST(qa_parallel_to_serial_vcc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_parallel_to_serial_vcc.py)
GR_ADD_TEST(qa_symbols_to_bits_cb ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_symbols_to_bits_cb.py)
GR_ADD_TEST(qa_preamble_insertion_vcvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_preamble_insertion_vcvc.py)
GR_ADD_TEST(qa_frame_sync_cc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_frame_sync_cc.py)
GR_ADD_TEST(qa_coarse_cfo_correction ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_coarse_cfo_correction.py)
GR_ADD_TEST(qa_rx_polyphase_cvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_rx_polyphase_cvc.py)
GR_ADD_TEST(qa_rx_sdft_cvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_rx_sdft_cvc.py)
GR_ADD_TEST(qa_tx_sdft_vcc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_tx_sdft_vcc.py)
GR_ADD_TEST(qa_rx_domain_cvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_rx_domain_cvc.py)
GR_ADD_TEST(qa_frame_generator_bvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_frame_generator_bvc.py)
GR_ADD_TEST(qa_deframer_vcb ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_deframer_vcb.py)
GR_ADD_TEST(qa_multichannel_frame_generator_bvc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_multichannel_frame_generator_bvc.py)
GR_ADD_TEST(qa_multichannel_deframer_vcb ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_multichannel_deframer_vcb.py)
GR_ADD_TEST(qa_multichannel_frame_sync_cc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_multichannel_frame_sync_cc.py)
GR_ADD_TEST(qa_time_freq_sync_cc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_time_freq_sync_cc.py)
GR_ADD_TEST(qa_phase_sync_cc ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_phase_sync_cc.py)
