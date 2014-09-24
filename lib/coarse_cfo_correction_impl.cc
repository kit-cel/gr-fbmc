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
#include "coarse_cfo_correction_impl.h"
#include <numeric>

namespace gr {
  namespace fbmc {

    coarse_cfo_correction::sptr
    coarse_cfo_correction::make(std::vector<int> channel_map)
    {
      return gnuradio::get_initial_sptr
        (new coarse_cfo_correction_impl(channel_map));
    }

    /*
     * The private constructor
     */
    coarse_cfo_correction_impl::coarse_cfo_correction_impl(std::vector<int> channel_map)
      : gr::block("coarse_cfo_correction",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      d_L = channel_map.size();
      d_channel_map.assign(&channel_map[0], &channel_map[0]+channel_map.size());

      // set up FFTW parameters
      d_interp_fac = 8; // 2 is the theoretical minimum
      d_nfft = d_interp_fac*d_L;
      d_buf = (fftwf_complex*) fftwf_malloc(sizeof(fftw_complex)*d_nfft);
      d_plan = fftwf_plan_dft_1d(d_nfft, d_buf, d_buf, FFTW_FORWARD, FFTW_ESTIMATE);

      // prepare the filter windows (frequency domain)
      d_delta = 2; // increasing this value makes the estimation better but reduces the lock range
      int upper_edge = 0; // highest used carrier
      int lower_edge = 0; // lowest used carrier
      for(int i=1; i<d_L; i++)
      {
        if(d_channel_map[i] == 0)
        {
          upper_edge = i;
          break;
        }
      }
      for(int i=d_L/2; i<d_L; i++)
      {
        if(d_channel_map[i] != 0)
        {
          lower_edge = i;
          break;
        }
      }
      if(upper_edge == 0 || lower_edge == 0 || std::accumulate(d_channel_map.begin(), d_channel_map.end(), 0) != upper_edge + (d_L - lower_edge))
        throw std::runtime_error("Channel map is assumed to be DC free and to have no empty carriers in the side bands");

      std::vector<int> tmp(d_nfft, 0);

      set_output_multiple(d_nfft);
    }

    /*
     * Our virtual destructor.
     */
    coarse_cfo_correction_impl::~coarse_cfo_correction_impl()
    {
    }

    void
    coarse_cfo_correction_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = d_nfft;
    }

    int
    coarse_cfo_correction_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

