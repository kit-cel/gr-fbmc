/* -*- c++ -*- */

#define FBMC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "fbmc_swig_doc.i"

%{
#include "fbmc/serialize_iq_cc.h"
#include "fbmc/apply_betas_cc.h"
%}


%include "fbmc/serialize_iq_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, serialize_iq_cc);
%include "fbmc/apply_betas_cc.h"
GR_SWIG_BLOCK_MAGIC2(fbmc, apply_betas_cc);
