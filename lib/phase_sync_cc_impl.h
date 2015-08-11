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

#ifndef INCLUDED_FBMC_PHASE_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_PHASE_SYNC_CC_IMPL_H

#include <fbmc/phase_sync_cc.h>

namespace gr {
  namespace fbmc {

    class phase_sync_cc_impl : public phase_sync_cc {
    private:
      int d_L;
      int d_nsym_frame;
      int d_search_window;
      float d_threshold;

      static const int d_overlap = 4;
      static const int d_num_subchannels = 4;
      int d_nsamp_frame; // number of samples per frame
      std::vector <std::vector<gr_complex> > d_mixed_preamble; // mixed preambles matching the subchannels
      gr_complex d_preamble_energy;
      uint8_t d_state;
      int d_samples_to_drop;
      int d_samples_to_return;
      int d_trailing_samples;
      gr_complex d_phi;

      static const uint8_t STATE_SEARCH = 0;
      static const uint8_t STATE_TRACK = 1;
      static const uint8_t STATE_DROP = 2;

      void prepare_mixed_preambles(std::vector <gr_complex> preamble, std::vector <float> taps);

      gr_complex calc_phase_offset(std::vector<gr_complex> corr_coefs);

      bool multichannel_detection(const gr_complex *buf, std::vector <gr_complex> &corr_coefs,
                                  std::vector <bool> &occupied_channels, int &index);

      void add_channel_occupation_tag(const std::vector <bool> &occupied_channels);

      void enter_search_state();

      void enter_track_state(int offset, const std::vector<gr_complex> &corr_coefs, const std::vector<bool> &occupied_channels);

      void enter_drop_state();

    public:
      phase_sync_cc_impl(int search_window, std::vector <gr_complex> preamble, float threshold, int L, int nsym_frame, std::vector<float> taps);

      ~phase_sync_cc_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PHASE_SYNC_CC_IMPL_H */

