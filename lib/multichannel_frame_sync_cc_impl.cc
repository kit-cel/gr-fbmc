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
#include <volk/volk.h>
#include "multichannel_frame_sync_cc_impl.h"

namespace gr {
  namespace fbmc {

    multichannel_frame_sync_cc::sptr
    multichannel_frame_sync_cc::make(int L, int nsym_frame, std::vector<gr_complex> preamble_sym, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new multichannel_frame_sync_cc_impl(L, nsym_frame, preamble_sym, step_size, threshold));
    }

    /*
     * The private constructor
     */
    multichannel_frame_sync_cc_impl::multichannel_frame_sync_cc_impl(int L, int nsym_frame, std::vector<gr_complex> preamble_sym, int step_size, float threshold)
      : gr::block("multichannel_frame_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
              d_L(L), d_nsym_frame(nsym_frame), d_preamble_sym(preamble_sym), d_step_size(step_size),
              d_threshold(threshold), d_nsamp_frame(d_L * d_nsym_frame / 2)
    {
      d_search_window_fixedlag = d_nsamp_frame;
      d_search_window_reference = d_L;
      d_search_buf = (gr_complex*) volk_malloc(sizeof(gr_complex)*(d_nsamp_frame+d_search_window_reference), volk_get_alignment());
      set_output_multiple(d_nsamp_frame); // length of one entire frame
    }

    /*
     * Our virtual destructor.
     */
    multichannel_frame_sync_cc_impl::~multichannel_frame_sync_cc_impl()
    {
    }

    void
    multichannel_frame_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = d_nsamp_frame + d_search_window_fixedlag + d_search_window_reference; // length of one entire frame plus search windows
    }

    int
    multichannel_frame_sync_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int consumed = 0;

      // I. detect frame via fixed lag correlation
      gr_complex corr_coef;
      bool proceed = false;
      while(consumed < d_search_window_reference - d_L) // make sure there is enough space for some reference correlation iterations
      {
        proceed = fixed_lag_correlation(in, corr_coef);
        if(proceed) { break; }
        else{ consumed += d_step_size; }
      }
      // proceed?
      if(!proceed)
      {
        consume_each(consumed);
        return 0;
      }

      // II. correct frequency offset
      memcpy(d_search_buf, in, sizeof(gr_complex)*(d_nsamp_frame+d_search_window_reference));
      double normalized_cfo = calc_cfo(corr_coef);
      correct_frequency_offset(normalized_cfo);

      // III. detect used subchannels via reference correlations
      std::vector<bool> occupied_channels;
      std::vector<gr_complex> corr_coefs;
      int pos = 0;
      while(pos < d_search_window_reference)
      {
        proceed = multichannel_detection(out, corr_coefs, occupied_channels); // returns true if at least one subchannel was detected
        if(proceed) { break; }
        else{ consumed += 1; }
        pos++;
      }
      // proceed?
      if(!proceed)
      {
        consume_each(consumed);
        return 0;
      }

      // IV. correct remaining phase offset
      correct_phase_offset(corr_coefs);

      // V. copy to output buffer and add tags denoting subchannel occupation
      memcpy(out, d_search_buf+pos, sizeof(gr_complex)*d_nsamp_frame);
      add_channel_occupation_tag(occupied_channels);

      // VI. consume and return entire frame
      consumed += d_nsamp_frame;
      consume_each(consumed);
      return d_nsamp_frame;
    }

  } /* namespace fbmc */
} /* namespace gr */

