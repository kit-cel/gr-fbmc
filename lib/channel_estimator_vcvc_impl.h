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

#ifndef INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_IMPL_H
#define INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_IMPL_H

#include <fbmc/channel_estimator_vcvc.h>
#include "interp2d.h"
#include "phase_helper.h"

namespace gr {
  namespace fbmc {

    class channel_estimator_vcvc_impl : public channel_estimator_vcvc {
    private:
      int d_subcarriers, d_pilot_timestep, d_o, d_frame_len, d_bands, d_curr_symbol, d_items_produced, d_lastpilot; 
      std::vector<float> d_taps;
      std::vector<int> d_pilot_carriers, d_spread_pilots, d_base_times, d_base_freqs;
      float d_pilot_amp;
      std::vector<gr_complex> d_curr_pilot, d_prev_pilot, d_pilots;
      std::vector<std::vector<gr_complex> > d_snippet;
      std::vector<gr_complex> d_curr_data;
      interp2d *d_interpolator;
      //phase_helper *d_helper;
			gr_complex* d_despread_temp;

      void interpolate_time(gr_complex*& out);
      void interpolate_freq(std::vector<gr_complex>::iterator estimate);
      inline void despread(gr_complex* out, const gr_complex* in, int noutput_items);
      //double fine_freq_sync();
      //double fine_time_sync();
      //std::vector<gr_complex> matrix_mean(Matrixc matrix, int axis);

    public:
      channel_estimator_vcvc_impl(int frame_len, int subcarriers, int overlap, int bands, std::vector<float> taps,
                                  float pilot_amp, int pilot_timestep,
                                  std::vector<int> pilot_carriers);

      ~channel_estimator_vcvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_IMPL_H */
