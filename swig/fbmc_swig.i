/* -*- c++ -*- */

#define FBMC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "fbmc_swig_doc.i"

%{
#include "../lib/fbmc_config.h"

#include "fbmc/apply_betas_vcvc.h"
#include "fbmc/serial_to_parallel_cvc.h"
#include "fbmc/serialize_iq_vcvc.h"
#include "fbmc/polyphase_filterbank_vcvc.h"
#include "fbmc/output_commutator_vcc.h"
#include "fbmc/simple_frame_generator_vcvc.h"
#include "fbmc/input_commutator_cvc.h"
#include "fbmc/combine_iq_vcvc.h"
#include "fbmc/parallel_to_serial_vcc.h"
#include "fbmc/symbols_to_bits_cb.h"
#include "fbmc/preamble_insertion_vcvc.h"
#include "fbmc/frame_sync_cc.h"
#include "fbmc/coarse_cfo_correction.h"
#include "fbmc/rx_polyphase_cvc.h"
#include "fbmc/rx_polyphase_kernel.h"
#include "fbmc/rx_sdft_cvc.h"
#include "fbmc/rx_sdft_kernel.h"
#include "fbmc/tx_sdft_vcc.h"
#include "fbmc/tx_sdft_kernel.h"
#include "fbmc/rx_domain_cvc.h"
#include "fbmc/rx_domain_kernel.h"
#include "fbmc/smt_kernel.h"
#include "fbmc/frame_generator_bvc.h"
#include "fbmc/deframer_vcb.h"
#include "fbmc/multichannel_frame_generator_bvc.h"
#include "fbmc/multichannel_deframer_vcb.h"
#include "fbmc/multichannel_frame_sync_cc.h"
#include "fbmc/time_freq_sync_cc.h"
#include "fbmc/phase_sync_cc.h"
#include "fbmc/subchannel_frame_generator_bvc.h"
#include "fbmc/sliding_fft_cvc.h"
#include "fbmc/channel_estimator_vcvc.h"
#include "fbmc/channel_equalizer_vcvc.h"
#include "fbmc/subchannel_deframer_vcb.h"
#include "fbmc/tx_dummy_mixer_cc.h"
#include "fbmc/cazac_sync_cc.h"
#include "fbmc/correlator_postprocessor_cf.h"
%}

%include "../lib/fbmc_config.h"

%include "fbmc/apply_betas_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, apply_betas_vcvc);
%include "fbmc/serial_to_parallel_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, serial_to_parallel_cvc);
%include "fbmc/serialize_iq_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, serialize_iq_vcvc);
%include "fbmc/polyphase_filterbank_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, polyphase_filterbank_vcvc);

%include "fbmc/output_commutator_vcc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, output_commutator_vcc);
%include "fbmc/simple_frame_generator_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, simple_frame_generator_vcvc);
%include "fbmc/input_commutator_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, input_commutator_cvc);

%include "fbmc/combine_iq_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, combine_iq_vcvc);
%include "fbmc/parallel_to_serial_vcc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, parallel_to_serial_vcc);
%include "fbmc/symbols_to_bits_cb.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, symbols_to_bits_cb);

%include "fbmc/preamble_insertion_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, preamble_insertion_vcvc);
%include "fbmc/frame_sync_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, frame_sync_cc);
%include "fbmc/coarse_cfo_correction.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, coarse_cfo_correction);
%include "fbmc/rx_polyphase_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, rx_polyphase_cvc);
%include "fbmc/rx_polyphase_kernel.h"
%include "fbmc/rx_sdft_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, rx_sdft_cvc);
%include "fbmc/rx_sdft_kernel.h"
%include "fbmc/tx_sdft_vcc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, tx_sdft_vcc);
%include "fbmc/tx_sdft_kernel.h"
%include "fbmc/rx_domain_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, rx_domain_cvc);
%include "fbmc/rx_domain_kernel.h"
%include "fbmc/smt_kernel.h"
%include "fbmc/frame_generator_bvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, frame_generator_bvc);
%include "fbmc/deframer_vcb.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, deframer_vcb);

%include "fbmc/multichannel_frame_generator_bvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, multichannel_frame_generator_bvc);
%include "fbmc/multichannel_deframer_vcb.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, multichannel_deframer_vcb);
%include "fbmc/multichannel_frame_sync_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, multichannel_frame_sync_cc);
%include "fbmc/time_freq_sync_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, time_freq_sync_cc);
%include "fbmc/phase_sync_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, phase_sync_cc);
%include "fbmc/subchannel_frame_generator_bvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, subchannel_frame_generator_bvc);


%include "fbmc/sliding_fft_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, sliding_fft_cvc);
%include "fbmc/channel_estimator_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, channel_estimator_vcvc);
%include "fbmc/channel_equalizer_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, channel_equalizer_vcvc);
%include "fbmc/subchannel_deframer_vcb.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, subchannel_deframer_vcb);
%include "fbmc/tx_dummy_mixer_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, tx_dummy_mixer_cc);
%include "fbmc/cazac_sync_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, cazac_sync_cc);
%include "fbmc/correlator_postprocessor_cf.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, correlator_postprocessor_cf);
