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
#include <fbmc/tx_sdft_kernel.h>

#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    tx_sdft_kernel::tx_sdft_kernel(const std::vector<float> &taps, int L, int symbols) :
        smt_kernel(taps, L), d_symbols(symbols)
    {
      // make sure we calculate the correct overlap size!
      int overlap = (taps.size() - 1) / L;
      if(overlap * L + 1 != taps.size()){
        throw std::runtime_error(
            "number of filter taps must be equal to L * overlap + 1!");
      }
      d_overlap = overlap;

      std::vector<float> my_taps(taps);
      my_taps.pop_back(); // remove last sample. Assuming PHYDYAS!

      int buff_len = overlap * L;
      d_taps_al = (float*) volk_malloc(sizeof(float) * buff_len,
                                       volk_get_alignment());
      std::vector<float>::iterator first_it = my_taps.begin();
      std::vector<float>::iterator last_it = my_taps.end();
      for(int i = 0; first_it != last_it; first_it++, i++){
        d_taps_al[i] = *first_it;
      }

      d_multiply_res = (gr_complex*) volk_malloc(sizeof(gr_complex) * buff_len,
                                                 volk_get_alignment());

      d_add_buf = (gr_complex*) volk_malloc(
          sizeof(gr_complex) * (buff_len + d_L), volk_get_alignment());
      memset(d_add_buf, 0, sizeof(gr_complex) * (buff_len + d_L));

      int fft_size = L;
      bool forward = false; // we want an IFFT
      int nthreads = 1; // may be altered if needed
      d_fft = new gr::fft::fft_complex(fft_size, forward, nthreads);
    }

    tx_sdft_kernel::~tx_sdft_kernel()
    {
      volk_free(d_multiply_res);
      volk_free(d_add_buf);
      delete d_fft;
    }

    int
    tx_sdft_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                 int noutput_items)
    {
      //int available_vectors = noutput_items / (d_L / 2);
      int finished_items = 0;
      for(int i = 0; i < d_symbols; i++){
        finished_items += process_one_vector(out, in);
        out += (d_L / 2);
        in += d_L;
      }

      memcpy(out, d_add_buf, sizeof(gr_complex) * d_L * (overlap() - 0.5));
      memset(d_add_buf, 0, sizeof(gr_complex) * ((d_overlap + 1) * d_L));
      return (finished_items- d_L/2) + d_L * overlap();
      //return finished_items;
    }

    inline int
    tx_sdft_kernel::process_one_vector(gr_complex* outbuf,
                                       const gr_complex* inbuf)
    {
      memcpy(d_fft->get_inbuf(), inbuf, sizeof(gr_complex) * d_L); // get data into FFT
      d_fft->execute(); // execute FFT. Get one vector.
      multiply_with_taps(d_multiply_res, d_fft->get_outbuf());
      return calculate_sum_result(outbuf, d_multiply_res);
    }

    /*
     * For this multiply [L, overlap and taps] must be available and known.
     * No need to pass them on as parameters.
     */
    inline void
    tx_sdft_kernel::multiply_with_taps(gr_complex* outbuf,
                                       const gr_complex* inbuf)
    {
      for(int i = 0; i < d_overlap; i++){
        volk_32fc_32f_multiply_32fc(outbuf + d_L * i, inbuf,
                                    d_taps_al + d_L * i, d_L);
      }
    }

    /*
     * use internal buffer to sum up result and copy completed samples to out buffer.
     */
    inline int
    tx_sdft_kernel::calculate_sum_result(gr_complex* outbuf,
                                         const gr_complex* res)
    {
      int num_samps = d_L / 2;
      /*
      // that would be the most efficient way but it seems like occasional glitches break this solution
      memset(outbuf + d_L * d_overlap - num_samps, 0, sizeof(gr_complex) * num_samps);
      volk_32f_x2_add_32f((float*) outbuf, (float*) outbuf, (float*) res,
                          2 * d_L * d_overlap);
      */
      volk_32f_x2_add_32f((float*) d_add_buf, (float*) d_add_buf, (float*) res,
                                2 * d_L * d_overlap);
      memcpy(outbuf, d_add_buf, sizeof(gr_complex) * num_samps);
      memmove(d_add_buf, d_add_buf + num_samps,
              sizeof(gr_complex) * d_L * d_overlap);
      return num_samps;
    }
  } /* namespace fbmc */
} /* namespace gr */

