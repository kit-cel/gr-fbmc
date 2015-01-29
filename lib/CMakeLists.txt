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
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND fbmc_sources
    fbmc_config.cc
    apply_betas_vcvc_impl.cc
    serial_to_parallel_cvc_impl.cc
    serialize_iq_vcvc_impl.cc
    polyphase_filterbank_vcvc_impl.cc
    output_commutator_vcc_impl.cc
    frame_generator_vcvc_impl.cc
    input_commutator_cvc_impl.cc
    combine_iq_vcvc_impl.cc
    parallel_to_serial_vcc_impl.cc
    symbols_to_bits_cb_impl.cc
    preamble_insertion_vcvc_impl.cc
    frame_sync_cc_impl.cc
    coarse_cfo_correction_impl.cc
    rx_polyphase_cvc_impl.cc
    rx_polyphase_kernel.cc
    rx_sdft_cvc_impl.cc
    rx_sdft_kernel.cc
    tx_sdft_vcc_impl.cc
    tx_sdft_kernel.cc
    rx_domain_cvc_impl.cc
    rx_domain_kernel.cc
    smt_kernel.cc )

set(fbmc_sources "${fbmc_sources}" PARENT_SCOPE)
if(NOT fbmc_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT fbmc_sources)

add_library(gnuradio-fbmc SHARED ${fbmc_sources})
target_link_libraries(gnuradio-fbmc ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} ${FFTW3F_LIBRARIES})
set_target_properties(gnuradio-fbmc PROPERTIES DEFINE_SYMBOL "gnuradio_fbmc_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-fbmc PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-fbmc
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})
list(APPEND test_fbmc_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_fbmc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_fbmc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/)

add_executable(test-fbmc ${test_fbmc_sources})

target_link_libraries(
  test-fbmc
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-fbmc
)

GR_ADD_TEST(test_fbmc test-fbmc)
