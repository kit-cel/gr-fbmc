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
#include <fbmc/rx_sdft_kernel.h>

#include <stdexcept>
#include <volk/volk.h>
#include <cstring> // necessary for memset
#include <algorithm>

namespace gr {
  namespace fbmc {

    rx_sdft_kernel::rx_sdft_kernel(
        const std::vector<float> &taps, int L) :
            smt_kernel(taps, L)
    {
      // make sure we calculate the correct overlap size!
      int overlap = (taps.size() - 1) / L;
      if(overlap * L + 1 != taps.size()){
        throw std::runtime_error("number of filter taps must be equal to L * overlap + 1!");
      }
      d_overlap = overlap;

      // we assume that PHYDYAS filters always have a zero tap at the end!
      if(taps.back() != 0.0f){
        throw std::runtime_error("Last element of PHYDYAS filter taps must be 0.0f!");
      }
      std::vector<float> my_taps(taps);
      my_taps.pop_back();

      // initialize all buffers correctly.
      int buff_len = overlap * L;

      // taps are expected to be provided from first on the left to last on the right.
      // samples are oldest on the left, newest on the right.
      // taps order reversed to match those conditions!
      d_taps_al = (float*) volk_malloc(sizeof(float) * buff_len, volk_get_alignment());
      std::vector<float>::reverse_iterator first_it = my_taps.rbegin();
      std::vector<float>::reverse_iterator last_it = my_taps.rend();
      for(int i = 0;first_it != last_it; first_it++, i++){
        d_taps_al[i] = *first_it;
      }

      d_multiply_res = (gr_complex*) volk_malloc(sizeof(gr_complex) * buff_len, volk_get_alignment());
      d_add_res = (gr_complex*) volk_malloc(sizeof(gr_complex) * L, volk_get_alignment());

      int fft_size = L;
      bool forward = false; // we want an IFFT
      int nthreads = 1; // may be altered if needed
      d_fft = new gr::fft::fft_complex(fft_size, forward, nthreads);
    }

    rx_sdft_kernel::~rx_sdft_kernel()
    {
      delete d_fft;
    }

    int
    rx_sdft_kernel::generic_work(gr_complex *out,
                                               const gr_complex *in,
                                               int noutput_items)
    {
      gr_complex* fft_inbuf = d_fft->get_inbuf();
      gr_complex* fft_outbuf = d_fft->get_outbuf();
      noutput_items = noutput_items - d_overlap + 1;
      for(int items = 0; items < noutput_items; items++){
        multiply_with_taps(d_multiply_res, in, d_L, d_overlap);
        add_overlaps(fft_inbuf, d_multiply_res, d_L, d_overlap);
        // reversing was previously done by input_commutator
        // profiling tools indicate reversing is VERY expensive!
        std::reverse(fft_inbuf, fft_inbuf + d_L);
        d_fft->execute();

        // some ideas to make it faster.
//        volk_32f_s32f_multiply_32f((float*)(fft_outbuf + 1), (float*)(fft_outbuf + 1), (-1.0f), (d_L - 1) * 2);
//        volk_32fc_s32fc_multiply_32fc(fft_outbuf + 1, fft_outbuf + 1, gr_complex(-1.0f, 0.0f), d_L - 1);

        memcpy(out, fft_outbuf, sizeof(gr_complex) * d_L); // write result to output
        in += d_L / 2;
        out += d_L;
      }


      return noutput_items;
    }

    inline void rx_sdft_kernel::multiply_with_taps(gr_complex *out_buff, const gr_complex *in_buff, int L, int overlap){
      volk_32fc_32f_multiply_32fc(out_buff, in_buff, d_taps_al, overlap * L);
    }

    inline void rx_sdft_kernel::add_overlaps(gr_complex *out_buff, const gr_complex *in_buff, int L, int overlap){
      int fL = 2 * L;
      memset(out_buff, 0, sizeof(gr_complex) * L);
      for(int i = 0; i < overlap; i++){
        volk_32f_x2_add_32f((float*) out_buff, (float*) out_buff, (float*) in_buff, fL);
        in_buff += L;
      }
    }


  } /* namespace fbmc */
} /* namespace gr */

