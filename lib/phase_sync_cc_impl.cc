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
#include "phase_sync_cc_impl.h"
#include <fbmc/tx_sdft_kernel.h>
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    phase_sync_cc::sptr
    phase_sync_cc::make(int search_window, std::vector <gr_complex> preamble, float threshold, int L, int nsym_frame,
                        std::vector <float> taps) {
      return gnuradio::get_initial_sptr
          (new phase_sync_cc_impl(search_window, preamble, threshold, L, nsym_frame, taps));
    }

    /*
     * The private constructor
     */
    phase_sync_cc_impl::phase_sync_cc_impl(int search_window, std::vector <gr_complex> preamble, float threshold, int L,
                                           int nsym_frame, std::vector <float> taps)
        : gr::block("phase_sync_cc",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
          d_search_window(search_window), d_threshold(threshold), d_L(L), d_nsym_frame(nsym_frame),
          d_state(STATE_SEARCH), d_samples_to_drop(0), d_samples_to_return(0), d_trailing_samples(0), d_phi(0)
    {
      d_nsamp_frame = d_nsym_frame * d_L / 2;
      prepare_mixed_preambles(preamble, taps);
      volk_32fc_x2_conjugate_dot_prod_32fc(&d_preamble_energy, &d_mixed_preamble[0][0], &d_mixed_preamble[0][0],
                                           d_mixed_preamble[0].size());
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    phase_sync_cc_impl::~phase_sync_cc_impl() {
    }

    void
    phase_sync_cc_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      ninput_items_required[0] =
          d_mixed_preamble[0].size() + d_search_window; // TODO: this is the minimum, but is it the optimum?
    }

    void
    phase_sync_cc_impl::prepare_mixed_preambles(std::vector < gr_complex > preamble, std::vector < float > taps) {
      // initialize kernel
      for (int i = 0; i < d_num_subchannels; i++) {
        tx_sdft_kernel kernel(taps, d_L);
        int nsym_preamble = preamble.size() / (d_L / d_num_subchannels);
        int nsym_sync = nsym_preamble + d_overlap;
        std::vector <gr_complex> symbols(d_L * nsym_sync, 0);
        d_mixed_preamble.push_back(std::vector<gr_complex>(d_L / 2 * nsym_sync, 0));
        for (int k = 0; k < nsym_preamble; k++) {
          memcpy(&symbols[k * d_L + i * d_L / d_num_subchannels], &preamble[k * d_L / d_num_subchannels],
                 sizeof(gr_complex) * d_L / d_num_subchannels);
        }
        kernel.generic_work(&d_mixed_preamble[i][0], &symbols[0], nsym_sync * d_L / 2);
      }
    }

    gr_complex
    phase_sync_cc_impl::calc_phase_offset(std::vector<gr_complex> corr_coefs)
    {
      // average the phases of the corr_coefs
      double angle = 0;
      for(int i=0; i<corr_coefs.size(); i++)
      {
        angle+= arg(corr_coefs[i]);
      }
      angle /= corr_coefs.size();
      return exp(gr_complex(0, -angle));
    }

    bool
    phase_sync_cc_impl::multichannel_detection(const gr_complex * buf, std::vector < gr_complex > &corr_coefs,
                                                    std::vector < bool > &occupied_channels, int & index)
    {
      index = 0;
      corr_coefs.clear();
      occupied_channels.clear();

      std::vector <std::vector<gr_complex> > corr_vec;
      std::vector <gr_complex> corr_sum(d_search_window, 0);
      gr_complex signal_energy = 0;
      gr_complex tmp = 0;
      for (int i = 0; i < d_search_window; i++) {
        corr_vec.push_back(std::vector<gr_complex>(d_num_subchannels, 0));
        volk_32fc_x2_conjugate_dot_prod_32fc(&signal_energy, buf + i, buf + i, d_mixed_preamble[0].size());
        for (int k = 0; k < d_num_subchannels; k++) {
          volk_32fc_x2_conjugate_dot_prod_32fc(&tmp, buf + i, &d_mixed_preamble[k][0], d_mixed_preamble[k].size());
          corr_vec[i][k] = tmp / sqrt(d_preamble_energy * signal_energy);
          corr_sum[i] += corr_vec[i][k];
        }
        corr_sum[i] = std::abs(corr_sum[i]);
        if (i > 0 and corr_sum[i].real() > corr_sum[index].real() and corr_sum[i].real() > d_threshold / d_num_subchannels) {
          index = i;
        }
      }

      if (index == 0 and corr_sum[0].real() < d_threshold /
                                              d_num_subchannels) // make sure index has been moved or is already at peak position
      {
        return false;
      }
      else {
        // find maximum
        float maxval = 0;
        std::vector <float> corr_abs;
        for (int i = 0; i < d_num_subchannels; i++) {
          corr_abs.push_back(std::abs(corr_vec[index][i]));
          if (corr_abs[i] > maxval) {
            maxval = corr_abs[i];
          }
        }
        for (int i = 0; i < d_num_subchannels; i++) {
          if (corr_abs[i] > maxval / 2) {
            occupied_channels.push_back(true);
            corr_coefs.push_back(corr_vec[index][i]);
          }
          else {
            occupied_channels.push_back(false);
          }
        }
        return true;
      }
    }

    void
    phase_sync_cc_impl::add_channel_occupation_tag(const std::vector <bool> &occupied_channels)
    {
      pmt::pmt_t pmt_vec = pmt::make_vector(d_num_subchannels, pmt::PMT_F);
      for(int i=0; i<d_num_subchannels; i++)
      {
        if(occupied_channels[i])
        {
          pmt::vector_set(pmt_vec, i, pmt::PMT_T);
        }
      }
      add_item_tag(0, nitems_written(0), pmt::mp("occupied_subchannels"), pmt_vec);
    }

    void
    phase_sync_cc_impl::enter_search_state()
    {
      std::cout << "phase_sync: enter SEARCH state" << std::endl;
      d_state = STATE_SEARCH;
    }

    void
    phase_sync_cc_impl::enter_track_state(int offset, const std::vector<gr_complex> &corr_coefs, const std::vector<bool> &occupied_channels)
    {
      d_state = STATE_TRACK;
      d_phi = calc_phase_offset(corr_coefs);
      add_channel_occupation_tag(occupied_channels);
      d_samples_to_return = d_nsamp_frame;
      d_trailing_samples = d_search_window - offset;
      std::cout << "phase_sync: detect preamble @" << nitems_read(0) + offset << ", phi=" << d_phi << ", enter TRACK state" << std::endl;
    }

    void
    phase_sync_cc_impl::enter_drop_state()
    {
      std::cout << "phase_sync: enter DROP state" << std::endl;
      d_state = STATE_DROP;
      d_state = STATE_DROP;
      d_samples_to_drop = d_nsamp_frame;
    }

    int
    phase_sync_cc_impl::general_work(int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int consumed = 0;
      int produced = 0;

//      std::cout << "phase_sync: work() called, state=" << int(d_state) << std::endl;

      if (d_state == STATE_SEARCH) {
        std::vector <bool> occupied_channels;
        std::vector <gr_complex> corr_coefs;
        int index = 0;
        bool preamble_detected = multichannel_detection(in, corr_coefs, occupied_channels, index);
        if(preamble_detected)
        {
          enter_track_state(index, corr_coefs, occupied_channels);
          consumed += index;
        }
        else
        {
          enter_drop_state();
          consumed += d_search_window;
        }
      }

      if(d_state == STATE_TRACK)
      {
        if(d_samples_to_return > 0) { // return frame
          int max_items_return = std::min(d_samples_to_return, std::min(ninput_items[0] - consumed, noutput_items));
          volk_32fc_s32fc_multiply_32fc(out, in + consumed, d_phi, max_items_return);
          consumed += max_items_return;
          produced += max_items_return;
          d_samples_to_return -= max_items_return;
//          std::cout << "phase_sync: samples left to return: " << d_samples_to_return << "/" << d_nsamp_frame << std::endl;
        }
        if(d_samples_to_return == 0 && d_trailing_samples > 0) // drop trailing samples
        {
          int max_items_drop = std::min(d_trailing_samples, ninput_items[0] - consumed);
          consumed += max_items_drop;
          d_trailing_samples -= max_items_drop;
        }
        if(d_samples_to_return == 0 && d_trailing_samples == 0) // frame returned, trailing samples dropped -> go back to SEARCH state
        {
          enter_search_state();
        }
      }
      else if(d_state == STATE_DROP)
      {
        int max_items = std::min(d_samples_to_drop, ninput_items[0]-consumed);
        consumed += max_items;
        d_samples_to_drop -= max_items;
//        std::cout << "phase_sync: samples left to drop: " << d_samples_to_drop << "/" << d_nsamp_frame << std::endl;
        if(d_samples_to_drop == 0)
        {
          enter_search_state();
        }
      }
      else
      {
        throw std::runtime_error("Invalid state");
      }

//      std::cout << "phase_sync: consume " << consumed << ", return " << produced << std::endl;
      consume_each(consumed);
      return produced;
    }

  } /* namespace fbmc */
} /* namespace gr */

