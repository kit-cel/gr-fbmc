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

#ifndef INCLUDED_FBMC_MULTICHANNEL_FRAME_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_MULTICHANNEL_FRAME_SYNC_CC_IMPL_H

#include <fbmc/multichannel_frame_sync_cc.h>

namespace gr {
  namespace fbmc {

    class multichannel_frame_sync_cc_impl : public multichannel_frame_sync_cc {
    private:
      int d_L; // number of subcarriers including all 4 subchannels
      int d_nsym_frame; // symbols (of length L/2) per frame
      std::vector < std::vector<gr_complex> > d_mixed_preamble; // mixed preambles matching the subchannels
      gr_complex d_preamble_energy;
      int d_step_size;
      float d_threshold; // threshold for correlation
      static const int d_overlap = 4;
      static const int d_num_subchannels = 4;
      int d_search_window_fixedlag;
      int d_fixedlag_lookahead;
      int d_search_window_reference;
      gr_complex *d_search_buf;
      int d_nsamp_frame; // number of samples per frame

      void prepare_mixed_preambles(std::vector<gr_complex> preamble, std::vector<float> taps);

//      void interpolate_preamble(std::vector<gr_complex> preamble);

      double calc_cfo(gr_complex c);

      void correct_frequency_offset(double cfo_norm);

      void correct_phase_offset(std::vector <gr_complex> corr_coefs);

      bool fixed_lag_correlation(const gr_complex *in, gr_complex &corr_coef);

      bool multichannel_detection(gr_complex *buf, std::vector<gr_complex>& corr_coefs,
                                  std::vector<bool>& occupied_channels);

      void add_channel_occupation_tag(const std::vector <bool> &occupied_channels);

    public:
      multichannel_frame_sync_cc_impl(int L, int nsym_frame, std::vector <gr_complex> preamble_sym,
                                      std::vector<float> taps, int step_size, float threshold);

      ~multichannel_frame_sync_cc_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_MULTICHANNEL_FRAME_SYNC_CC_IMPL_H */

