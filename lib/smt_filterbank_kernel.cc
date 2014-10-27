/* -*- c++ -*- */
/* 
 * Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <fbmc/smt_filterbank_kernel.h>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <volk/volk.h>
#include <fftw3.h>
#include <cstring>

namespace gr {
  namespace fbmc {

    smt_filterbank_kernel::smt_filterbank_kernel(std::vector<float> &taps,
                                                 int L) :
        filterbank(
            std::vector<std::vector<float> >(L, std::vector<float>(1, 0.0f))), d_L(
            L)
    {
      if(d_L < 2 || d_L % 2 != 0){
        throw std::runtime_error("L has to be even and >= 2!");
      }

      // calculate actual taps and distribution for taps
      set_taps(taps);
      // initialize buffers for filters.
      initialize_branch_buffers(L, d_fir_filters[0]->ntaps());

      // just like FFT block input and output buffers are used and copied out finally
      d_fft_in_buf = (gr_complex*) volk_malloc(sizeof(gr_complex) * d_L,
                                               volk_get_alignment());
      d_fft_out_buf = (gr_complex*) volk_malloc(sizeof(gr_complex) * d_L,
                                                volk_get_alignment());

      // create FFTW plan.
      d_fft_plan = fftwf_plan_dft_1d(
          d_L, reinterpret_cast<fftwf_complex *>(d_fft_in_buf),
          reinterpret_cast<fftwf_complex *>(d_fft_out_buf), FFTW_BACKWARD,
          FFTW_MEASURE);
    }

    smt_filterbank_kernel::~smt_filterbank_kernel()
    {
      volk_free(d_fft_in_buf);
      volk_free(d_fft_out_buf);
      for(int i = 0; i < d_buffers.size(); i++){
        volk_free(d_buffers[i]);
      }
    }

    void
    smt_filterbank_kernel::set_taps(std::vector<float> &taps)
    {
      // this method sanitizes the given taps and sets them for the filterbank afterwards
      // pad the prototype to an integer multiple length of L
      while(taps.size() % d_L != 0){
        taps.push_back(0.0f);
      }

      int num_branch_taps = ((taps.size() / d_L) * 2) - 1;

      d_prototype_taps.clear(); // make sure it is empty!
      for(int i = 0; i < d_L; i++){ // allocate all branches/taps
        std::vector<float> v(num_branch_taps, 0.0f);
        d_prototype_taps.push_back(v);
      }

      int tap_num = 0;
      for(int l = 0; l < num_branch_taps; l++){ // l is tap number
        for(int i = 0; i < d_L; i++){ // i is branch number
          if(l % 2 == 0){
            d_prototype_taps[i][l] = taps[tap_num++];
          }
          else{
            d_prototype_taps[i][l] = 0.0f;
          }
        }
      }

      filterbank::set_taps(d_prototype_taps);
    }

    void
    smt_filterbank_kernel::initialize_branch_buffers(int L, int ntaps)
    {
      d_buffers.clear();
      gr_complex* buf = (gr_complex*) volk_malloc(sizeof(gr_complex) * ntaps, volk_get_alignment());
      d_buffers.resize(L);
      for(int i = 0; i < L; i++){
        d_buffers[i] = (gr_complex*) volk_malloc(sizeof(gr_complex) * ntaps, volk_get_alignment());
        memset(d_buffers[i], 0, sizeof(gr_complex) * ntaps);
      }
    }

    inline void
    smt_filterbank_kernel::update_branch_buffer(gr_complex in_sample, int branch)
    {
      // for now move all samples by one
      // left or right shift is open to debate.
      // fir_filter seems to expect left shift and new samples on the right.
      memmove(d_buffers[branch], d_buffers[branch] + 1, sizeof(gr_complex) * (d_fir_filters[branch]->ntaps() - 1));
      d_buffers[branch][d_fir_filters[branch]->ntaps() - 1] = in_sample;
    }

    inline gr_complex
    smt_filterbank_kernel::filter_branch(gr_complex in_sample, int branch)
    {
      // method gets new input sample and the branch number
      // put input sample to front of branch buffer
      update_branch_buffer(in_sample, branch);

      // do filtering.
      gr_complex result = d_fir_filters[branch]->filter(d_buffers[branch]);

      return result;
    }

    int
    smt_filterbank_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                        int noutput_items)
    {
      for(int items = 0; items < noutput_items; items++){
        for(int i = 0; i < d_L; i++){ // go thru filter arms!
          d_fft_in_buf[i] = filter_branch(*(in + (d_L - 1) - i), i);
        }

        // do fft.
        fftwf_execute(d_fft_plan);
        memcpy(out, d_fft_out_buf, sizeof(gr_complex) * d_L);
        in += d_L / 2;
        out += d_L;
      }

      return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

