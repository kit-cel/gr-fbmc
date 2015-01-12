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

    rx_domain_kernel::rx_domain_kernel(std::vector<float> taps, int L, int overlap):
        d_taps(taps), d_L(L), d_overlap(overlap)
    {
      int fft_size = overlap * L;
      bool forward = false; // we want an IFFT
      int nthreads = 1; // may be altered if needed
      d_fft = new gr::fft::fft_complex(fft_size, forward, nthreads);

      d_equalized = (gr_complex*) volk_malloc(sizeof(gr_complex) * (overlap * L + 2 * overlap), volk_get_alignment());

      d_num_al_taps = std::ceil(float(taps.size()) / float(volk_get_alignment())) * volk_get_alignment();
      d_taps_al = (gr_complex*) volk_malloc(sizeof(gr_complex) * d_num_al_taps, volk_get_alignment());
      memset(d_taps_al, 0, sizeof(gr_complex) * d_num_al_taps);
      for(int i = 0; i < taps.size(); i++){
        d_taps_al[i] = taps[i];
      }
    }

    rx_domain_kernel::~rx_domain_kernel()
    {
    }

    int
    rx_domain_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                   int noutput_items)
    {
      memcpy(d_fft->get_inbuf(), in, sizeof(gr_complex) * d_overlap * d_L);
      d_fft->execute();
      equalize(d_equalized, d_fft->get_outbuf());
      apply_taps(out, in, d_L);
      std::cout << "work with nout = " << noutput_items << std::endl;
      return 1;
    }

    void
    rx_domain_kernel::equalize(gr_complex* outbuf, const gr_complex* inbuf)
    {
      // copy FFT'ed vector
      // this would be a good starting point for actual equalization code.
      memcpy(outbuf + d_overlap, inbuf, sizeof(gr_complex) * d_overlap * d_L);
      // copy last part of FFT to front
      memcpy(outbuf, inbuf + d_overlap * d_L - d_overlap, sizeof(gr_complex) * d_overlap);
      // copy last part of FFT to front. It's all cyclic.
      memcpy(outbuf + d_overlap + d_overlap * d_L, inbuf, sizeof(gr_complex) * d_overlap);
    }

    void
    rx_domain_kernel::apply_taps(gr_complex* outbuf, const gr_complex* inbuf,
                                 const int outbuf_len)
    {
      for(int i = 0; i < outbuf_len; i++){
        volk_32fc_x2_dot_prod_32fc(outbuf, inbuf, d_taps_al, d_num_al_taps);
        outbuf++;
        inbuf += d_overlap;
      }
    }

  } /* namespace fbmc */
} /* namespace gr */
