/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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
#include <fbmc/rx_domain_kernel.h>

#include <iostream>
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    rx_domain_kernel::rx_domain_kernel(std::vector<float> taps, int L):
        d_taps(taps), d_L(L)
    {
      int overlap = (taps.size() + 1) / 2;
      if(overlap * 2 - 1 != taps.size()){
        throw std::runtime_error("number of frequency domain taps not equal to 2 * overlap -1 !!");
      }
      d_overlap = overlap;
      int fft_size = overlap * L;
      bool forward = true; // we want an IFFT
      int nthreads = 1; // may be altered if needed
      d_fft = new gr::fft::fft_complex(fft_size, forward, nthreads);

      d_equalized = (gr_complex*) volk_malloc(sizeof(gr_complex) * (overlap * L + overlap), volk_get_alignment());

      int volk_complex_al_items = volk_get_alignment() / sizeof(gr_complex);
      int num_taps = taps.size();
      int al_multiple = std::ceil(float(num_taps) / float(volk_complex_al_items));
      d_num_al_taps = al_multiple * volk_complex_al_items;
      std::cout << "volk_complex: " << volk_complex_al_items << ", num_taps: " << num_taps << ", multiple: " << al_multiple << ", result: " << d_num_al_taps << std::endl;
      d_taps_al = (float*) volk_malloc(sizeof(float) * d_num_al_taps, volk_get_alignment());
      memset(d_taps_al, 0, sizeof(float) * d_num_al_taps);
      for(int i = 0; i < taps.size(); i++){
        d_taps_al[i + 1] = taps[i];
      }
    }

    rx_domain_kernel::~rx_domain_kernel()
    {
    }

    int
    rx_domain_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                   int noutput_items)
    {
      int fft_size = d_overlap * d_L;
      std::cout << "inbuffer\n";
      containsNaN(in, fft_size);
      memcpy(d_fft->get_inbuf(), in, sizeof(gr_complex) * fft_size);
      d_fft->execute();
      std::cout << "fft_buffer\n";
      containsNaN(d_fft->get_outbuf(), fft_size);
//      memcpy(out, d_fft->get_outbuf(), sizeof(gr_complex) * fft_size);
      equalize(d_equalized, d_fft->get_outbuf());
      std::cout << "equalized buffer\n";
      containsNaN(d_equalized, fft_size + d_overlap);
      apply_taps(out, d_equalized, d_L);
      std::cout << "out buffer\n";
      containsNaN(out, d_L);
      return 1;
    }

    void
    rx_domain_kernel::equalize(gr_complex* outbuf, const gr_complex* inbuf)
    {
      // copy FFT'ed vector
      // this would be a good starting point for actual equalization code.
      memcpy(outbuf + d_overlap, inbuf, sizeof(gr_complex) * d_overlap * d_L);
      // copy last part of FFT to front
      memcpy(outbuf, outbuf + d_overlap * d_L, sizeof(gr_complex) * d_overlap);
    }

    void
    rx_domain_kernel::apply_taps(gr_complex* outbuf, const gr_complex* inbuf,
                                 const int outbuf_len)
    {
      // this loop could select used subcarriers only. Thus drop unnecessary calculations.
      for(int i = 0; i < outbuf_len; i++){
        volk_32fc_32f_dot_prod_32fc(outbuf, inbuf, d_taps_al, d_num_al_taps);
        if(std::isnan((*outbuf).real()) ||std::isnan((*outbuf).imag())){
          std::cout << "isNaN: " << i << "\n" << containsNaN(inbuf, d_num_al_taps);

        }
        outbuf++;
        inbuf += d_overlap;
      }
    }

    bool
    rx_domain_kernel::containsNaN(const gr_complex* buf, const int vec_length)
    {
      bool is_broken = false;
      for(int i = 0; i < vec_length; i++){
        is_broken = std::isnan(buf[i].real());
        if(is_broken){break;}
        is_broken = std::isnan(buf[i].imag());
        if(is_broken){break;}
      }
      if(is_broken){
        std::cout << "NaN: " << is_broken << ", length = " << vec_length << std::endl;
        for(int i = 0; i < vec_length; i++){
          std::cout << buf[i] << ", ";
        }
        std::cout << "\n";
      }
      return is_broken;
    }

  } /* namespace fbmc */
} /* namespace gr */


