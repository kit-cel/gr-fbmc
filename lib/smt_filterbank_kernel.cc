/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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
      std::cout << "smt_filterbank_kernel::ctor\n";
      set_taps(taps);

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
    }

    void
    smt_filterbank_kernel::set_taps(std::vector<float> &taps)
    {
      // this method sanitizes the given taps and sets them for the filterbank afterwards
      std::cout << "smt_filterbank_kernel::set_taps()\n";
      std::cout << "set_taps(n = " << taps.size() << ")\n";
      // pad the prototype to an integer multiple length of L
      while(taps.size() % d_L != 0){
        taps.push_back(0.0f);
      }
      std::cout << "padded(n = " << taps.size() << ")\n";
      int num_branch_taps = ((taps.size() / d_L) * 2) - 1;
      std::cout << "branches = " << d_L << std::endl;
      std::cout << "branch_taps(n = " << num_branch_taps << ")\n";
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

    int
    smt_filterbank_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                        int noutput_items)
    {
      for(int i = 0; i < d_L; i++){ // go thru filter arms!
        d_fft_in_buf[i] = d_fir_filters[i]->filter(in + (d_L - 1) - i);
        std::cout << d_fft_in_buf[i] << ", ";
//        out[i] = in[(d_L - 1) - i];
      }
      std::cout << std::endl;
      // do fft.
      fftwf_execute(d_fft_plan);
      memcpy(out, d_fft_out_buf, sizeof(gr_complex) * d_L);

      return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

