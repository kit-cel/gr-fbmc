/* -*- c++ -*- */

#define FBMC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "fbmc_swig_doc.i"

%{
#include "fbmc/apply_betas_vcvc.h"
#include "fbmc/serial_to_parallel_cvc.h"
#include "fbmc/serialize_iq_vcvc.h"
#include "fbmc/polyphase_filterbank_vcvc.h"
#include "fbmc/output_commutator_vcc.h"
#include "fbmc/frame_generator_vcvc.h"
#include "fbmc/input_commutator_cvc.h"
%}


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
%include "fbmc/frame_generator_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, frame_generator_vcvc);
%include "fbmc/input_commutator_cvc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, input_commutator_cvc);