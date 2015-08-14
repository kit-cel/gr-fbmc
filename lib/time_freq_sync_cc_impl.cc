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

    time_freq_sync_cc::sptr
    time_freq_sync_cc::make(int L, float threshold, int nsym_frame, int stepsize, int additional_samps) {
      return gnuradio::get_initial_sptr
          (new time_freq_sync_cc_impl(L, threshold, nsym_frame, stepsize, additional_samps));
    }

    /*
     * The private constructor
     */
    time_freq_sync_cc_impl::time_freq_sync_cc_impl(int L, float threshold, int nsym_frame, int stepsize, int additional_samps)
        : gr::block("time_freq_sync_cc",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
          d_L(L), d_threshold(threshold), d_nsym_frame(nsym_frame), d_nsamp_frame(L * nsym_frame / 2),
          d_lookahead(3 * L / 2), d_state(STATE_SEARCH), d_phi(0.0), d_nsamp_remaining(L * nsym_frame / 2),
          d_cfo(0.0), d_stepsize(stepsize), d_additional_samps(additional_samps), d_corrbuf_num_sum(0),
          d_corrbuf_denom1_sum(0), d_corrbuf_denom2_sum(0)
    {
      if(d_L % d_stepsize != 0)
      {
        throw std::runtime_error("L % stepsize must be 0");
      }
      d_corrbuf_len = d_L/d_stepsize;
      d_corrbuf_num.set_capacity(d_corrbuf_len);
      d_corrbuf_denom1.set_capacity(d_corrbuf_len);
      d_corrbuf_denom2.set_capacity(d_corrbuf_len);

      enter_search_state();

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
    time_freq_sync_cc_impl::enter_search_state()
    {
//      std::cout << "time_freq_sync: enter SEARCH state" << std::endl;
      d_state = STATE_SEARCH;
      d_corrbuf_num_sum = 0;
      d_corrbuf_num.clear();
      d_corrbuf_denom1_sum = 0;
      d_corrbuf_denom2_sum = 0;
      d_corrbuf_denom1.clear();
      d_corrbuf_denom2.clear();
    }

    void
    time_freq_sync_cc_impl::enter_track_state(int offset)
    {
//      std::cout << "time_freq_sync: enter TRACK state" << std::endl;
      d_state = STATE_TRACK;
      d_cfo = -1.0 / (2 * M_PI * d_L) * arg(corrbuf());
      d_phi = 0.0;
//      std::cout << "time_freq_sync: frame detected! |rho| = " << std::abs(corrbuf()) << ", cfo = " << d_cfo << ", in@" << nitems_read(0)+offset << std::endl;
      d_nsamp_remaining = d_nsamp_frame + d_additional_samps; // return a little more to avoid cutting off the end of the frame in case of an early sync
//      std::cout << "time_freq_sync: frame detected, put tag out@" << nitems_written(0) + offset << std::endl;
      add_item_tag(0, nitems_written(0), pmt::mp("frame_start"), pmt::from_long(nitems_written(0) + offset));
    }

    int
    time_freq_sync_cc_impl::fill_buffer(const gr_complex *inbuf)
    {
      gr_complex* ptr = (gr_complex*) inbuf;
      while(d_corrbuf_num.size() < d_corrbuf_len - 1) // fill the buffer until only one "packet" is missing
      {
        add_step_to_buffer(ptr);
        ptr += d_stepsize;
      }
      return ptr - inbuf;
    }

    void
    time_freq_sync_cc_impl::add_step_to_buffer(const gr_complex *inbuf)
    {
      gr_complex num = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&num, inbuf , inbuf + d_L, d_stepsize);
      d_corrbuf_num.push_back(num);
      d_corrbuf_num_sum += num;

      // denominator
      gr_complex denom1 = 0;
      gr_complex denom2 = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&denom1, inbuf, inbuf, d_stepsize);
      volk_32fc_x2_conjugate_dot_prod_32fc(&denom2, inbuf + d_L, inbuf + d_L, d_stepsize);
      d_corrbuf_denom1.push_back(denom1);
      d_corrbuf_denom2.push_back(denom2);
      d_corrbuf_denom1_sum += denom1;
      d_corrbuf_denom2_sum += denom2;
    }

    void
    time_freq_sync_cc_impl::remove_step_from_buffer()
    {
      // numerator
      d_corrbuf_num_sum -= d_corrbuf_num[0];
      d_corrbuf_num.pop_front();

      // denominator
      d_corrbuf_denom1_sum -= d_corrbuf_denom1[0];
      d_corrbuf_denom2_sum -= d_corrbuf_denom2[0];
      d_corrbuf_denom1.pop_front();
      d_corrbuf_denom2.pop_front();
    }

    gr_complex
    time_freq_sync_cc_impl::corrbuf()
    {
      if(!d_corrbuf_num.full())
      {
        throw std::runtime_error("Invalid call to corrbuf, buffers not filled");
      }
      return d_corrbuf_num_sum/std::sqrt(d_corrbuf_denom1_sum*d_corrbuf_denom2_sum);
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

      if (d_state == STATE_SEARCH) // proceed by stepsize samples
      {
        gr_complex* corrstart = (gr_complex*) in + d_lookahead + d_corrbuf_num.size()*d_stepsize;
        int offset = d_lookahead + d_corrbuf_num.size() * d_stepsize;
        if(d_corrbuf_num.size() < d_corrbuf_len - 1)
        {
          corrstart += fill_buffer(&in[offset]);
        }
        corrstart = (gr_complex*) in + d_lookahead + d_L - d_stepsize;
        offset = d_lookahead + d_L - d_stepsize;
        int nsteps = (ninput_items[0] - offset)/d_stepsize;
        for(int i=0; i<nsteps; i++)
        {
          add_step_to_buffer(&in[offset]);
          offset += d_stepsize;
          corrstart += d_stepsize;
          if(std::abs(corrbuf()) > d_threshold) // frame detected
          {
            enter_track_state(offset-d_lookahead-d_L);
            break;
          }
          else
          {
            remove_step_from_buffer();
          }
        }
        consumed = offset - d_lookahead - d_L;
      }
      else if (d_state == STATE_TRACK)
      {
        int max_items = std::min(std::min(ninput_items[0], noutput_items), d_nsamp_remaining);
        for (int i = 0; i < max_items; i++) {
          out[i] = in[i] * exp(gr_complex(0, -2 * M_PI * d_cfo * i + d_phi));
        }
        d_phi += -2 * M_PI * d_cfo * max_items;
        produced += max_items;
        if(d_nsamp_remaining - max_items < d_additional_samps){ // do not consume the additional samples to avoid missing the next frame
          consumed += std::max(0, d_nsamp_remaining - d_additional_samps);
        }
        else{
          consumed += max_items;
        }
        d_nsamp_remaining -= max_items;
        if (d_nsamp_remaining == 0) {
          enter_search_state();
        }
      }

      consume_each(consumed);
      return produced;
    }

  } /* namespace fbmc */
} /* namespace gr */

