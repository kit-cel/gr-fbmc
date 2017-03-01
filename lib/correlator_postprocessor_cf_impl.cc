/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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
#include "correlator_postprocessor_cf_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    correlator_postprocessor_cf::sptr
    correlator_postprocessor_cf::make(float offset, int window_len)
    {
      return gnuradio::get_initial_sptr
        (new correlator_postprocessor_cf_impl(offset, window_len));
    }

    /*
     * The private constructor
     */
    correlator_postprocessor_cf_impl::correlator_postprocessor_cf_impl(float offset, int window_len)
      : gr::sync_block("correlator_postprocessor_cf",
              gr::io_signature::make(5, 5, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float))),
              d_offset(offset),
              d_window_len(window_len),
              d_avg(0)
    {
      set_output_multiple(4*window_len);
    }

    /*
     * Our virtual destructor.
     */
    correlator_postprocessor_cf_impl::~correlator_postprocessor_cf_impl()
    {
    }

    int
    correlator_postprocessor_cf_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in[5] = {(gr_complex*) input_items[0], (gr_complex*) input_items[1], (gr_complex*) input_items[2], (gr_complex*) input_items[3], (gr_complex*) input_items[4]};
      float *out = (float *) output_items[0];

      int nitems_available = noutput_items - d_window_len - 1;
      for(int i = 0; i < nitems_available; i++)
      {
        float corr_power = 0;
        for(int n = 0; n < 4; n++)
        {
          corr_power += std::abs(in[n][i]);
        }
        d_avg = d_avg - std::pow(std::abs(in[4][i]), 2) + std::pow(std::abs(in[4][i+d_window_len]), 2); 
        out[i] = corr_power / (d_avg + d_offset);    
      }

      // Tell runtime system how many output items we produced.
      return nitems_available;
    }

  } /* namespace fbmc */
} /* namespace gr */

