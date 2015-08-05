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
#include "time_freq_sync_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    uint8_t STATE_STARTSEARCH = 0;
    uint8_t STATE_SEARCH = 1;
    uint8_t STATE_TRACK = 2;

    time_freq_sync_cc::sptr
    time_freq_sync_cc::make(int L, float threshold, int nsym_frame, int stepsize) {
      return gnuradio::get_initial_sptr
          (new time_freq_sync_cc_impl(L, threshold, nsym_frame, stepsize));
    }

    /*
     * The private constructor
     */
    time_freq_sync_cc_impl::time_freq_sync_cc_impl(int L, float threshold, int nsym_frame, int stepsize)
        : gr::block("time_freq_sync_cc",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
          d_L(L), d_threshold(threshold), d_nsym_frame(nsym_frame), d_nsamp_frame(L * nsym_frame / 2), d_corrbuf(0),
          d_lookahead(3 * L / 2), d_state(STATE_STARTSEARCH), d_phi(0.0), d_nsamp_remaining(L * nsym_frame / 2),
          d_cfo(0.0), d_stepsize(stepsize) {
      set_output_multiple(d_lookahead + 2 * d_L); // minimum
    }

    /*
     * Our virtual destructor.
     */
    time_freq_sync_cc_impl::~time_freq_sync_cc_impl() {
    }

    void
    time_freq_sync_cc_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      ninput_items_required[0] = noutput_items;
    }

    void
    time_freq_sync_cc_impl::enter_track_state()
    {
      d_state = STATE_TRACK;
      d_cfo = -1.0 / (2 * M_PI * d_L) * arg(d_corrbuf);
      d_phi = 0.0;
      d_nsamp_remaining = d_nsamp_frame;
    }

    void
    time_freq_sync_cc_impl::corr_remove_old(const gr_complex *buf, int pos)
    {
      for(int i=0; i<d_stepsize; i++)
      {
        d_corrbuf -= std::conj(buf[d_lookahead + pos + i]) * buf[d_lookahead + pos + d_L + i];
      }
    }

    void
    time_freq_sync_cc_impl::corr_add_new(const gr_complex *buf, int pos)
    {
      for(int i=0; i<d_stepsize; i++)
      {
        d_corrbuf += std::conj(buf[d_lookahead + pos + d_L]) * buf[d_lookahead + pos + 2 * d_L];
      }
    }

    int
    time_freq_sync_cc_impl::general_work(int noutput_items,
                                         gr_vector_int &ninput_items,
                                         gr_vector_const_void_star &input_items,
                                         gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int consumed = 0;
      int produced = 0;
      int available = std::min(ninput_items[0], noutput_items);

      if (d_state == STATE_STARTSEARCH) {
        gr_complex acorr, xcorr;
        volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, in + d_lookahead, in + d_lookahead + d_L, d_L);
        volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, in + d_lookahead, in + d_lookahead, 2 * d_L);
        d_corrbuf = gr_complex(2, 0) * xcorr / acorr;
        if (std::abs(d_corrbuf) > d_threshold) {
          std::cout << "time_freq_sync: frame detected @" << nitems_read(0) + consumed << std::endl;
          enter_track_state();
        }
        else {
          corr_remove_old(in, consumed);
          d_state = STATE_SEARCH;
          consumed += d_stepsize;
          available -= d_stepsize;
        }
      }
      while (available > d_stepsize) {
        if (d_state == STATE_SEARCH) {
          corr_add_new(in, consumed);
          if (std::abs(d_corrbuf) > d_threshold) {
            std::cout << "time_freq_sync: frame detected @" << nitems_read(0) + consumed << std::endl;
            enter_track_state();
          }
          else {
            corr_remove_old(in, consumed);
            consumed += d_stepsize;
            available -= d_stepsize;
          }
        }
        else if (d_state == STATE_TRACK) {
          int max_items = std::min(available, d_nsamp_remaining);
          for (int i = 0; i < max_items; i++) {
            out[i + produced] = in[consumed + i] * exp(gr_complex(0, -2 * M_PI * d_cfo * i + d_phi));
          }
          d_phi += -2 * M_PI * d_cfo * max_items;
          produced += max_items;
          available -= max_items;
          consumed += max_items;
          d_nsamp_remaining -= max_items;
          if (d_nsamp_remaining == 0) {
            d_state = STATE_STARTSEARCH;
            break;
          }
        }
        else {
          throw std::runtime_error("Invalid state");
        }
      }

      consume_each(consumed);
      return produced;
    }

  } /* namespace fbmc */
} /* namespace gr */

