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
      d_fft = setup_fft(L, overlap);
      d_equalized = (gr_complex*) volk_malloc(sizeof(gr_complex) * (overlap * L + overlap), volk_get_alignment());
      d_taps_al = setup_taps_array(taps);
    }

    rx_domain_kernel::~rx_domain_kernel()
    {
      delete d_fft;
      volk_free(d_equalized);
      volk_free(d_taps_al);
    }

    gr::fft::fft_complex*
    gr::fbmc::rx_domain_kernel::setup_fft(int L, int overlap)
    {
      int fft_size = overlap * L;
      bool forward = true; // we want an IFFT
      int nthreads = 1; // may be altered if needed
      return new gr::fft::fft_complex(fft_size, forward, nthreads);
    }

    float*
    gr::fbmc::rx_domain_kernel::setup_taps_array(std::vector<float> taps)
    {
      int volk_float_al_items = volk_get_alignment() / sizeof(float);
      int num_taps = taps.size();
      int al_multiple = std::ceil(
          float(num_taps) / float(volk_float_al_items));
      // this class member is set within setup and not returned like the array pointer.
      d_num_al_taps = al_multiple * volk_float_al_items;
      float* taps_al = (float*) volk_malloc(sizeof(float) * d_num_al_taps,
                                       volk_get_alignment());
      memset(taps_al, 0, sizeof(float) * d_num_al_taps);
      for(int i = 0; i < taps.size(); i++){
        taps_al[i + 1] = taps[i];
      }

      std::cout << "volk_float: " << volk_float_al_items
          << ", num_taps: " << num_taps
          << ", multiple: " << al_multiple
          << ", result: " << d_num_al_taps << std::endl;

      return taps_al;
    }

    std::vector<gr_complex>
    rx_domain_kernel::generic_work_python(const std::vector<gr_complex> &inbuf)
    {
      std::vector<gr_complex> outbuf;
      if(inbuf.size() < fft_size()) {
        std::cout << "Not enough input items to calculate at least 1 FFT!\n";
        return outbuf;
      }
      if(inbuf.size() % (d_L / 2) != 0){
        std::cout << "input - output ration not met!\n";
        return outbuf;
      }

      int noutput_items = 1 + (inbuf.size() - fft_size()) / (d_L / 2);
      outbuf.resize(noutput_items * d_L);

      std::cout << "requested items = " << noutput_items << ", inbuf.size() = " << inbuf.size()
          << std::endl;

//      // fancy new shit! Using data() member on vectors. C++11.
      int nout = generic_work(outbuf.data(), inbuf.data(), noutput_items);
      return outbuf;
    }

    int
    rx_domain_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                   int noutput_items)
    {
      const int fft_size = d_overlap * d_L;
      for(int i = 0; i < noutput_items; i++){
        memcpy(d_fft->get_inbuf(), in, sizeof(gr_complex) * fft_size);
        d_fft->execute();
        equalize(d_equalized, d_fft->get_outbuf());
        apply_taps(out, d_equalized, d_L);
        in += (d_L / 2);
        out += d_L;
      }

      return noutput_items;
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
        outbuf++;
        inbuf += d_overlap;
      }
    }

  } /* namespace fbmc */
} /* namespace gr */
