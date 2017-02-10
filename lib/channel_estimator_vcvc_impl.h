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
#include <Eigen/Dense>
#include "interp2d.h"
#include "phase_helper.h"

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrixf;
typedef Eigen::Matrix<gr_complex, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrixc;

namespace gr {
  namespace fbmc {

    class channel_estimator_vcvc_impl : public channel_estimator_vcvc {
    private:
      int d_subcarriers, d_pilot_timestep, d_o, d_frame_len, d_bands, d_curr_symbol;
      std::vector<float> d_taps;
      std::vector<int> d_pilot_carriers, d_spread_pilots;
      float d_pilot_amp;
      const Matrixf d_G = spreading_matrix();
      Matrixc d_curr_pilot;
      Matrixc d_R;
      Matrixc d_prev_pilot;
      interp2d *d_interpolator;
      phase_helper *d_helper;
      bool d_pilot_stored;

      Matrixf spreading_matrix();
      void interpolate_time(std::vector<Matrixc>& queue);
      void interpolate_freq(Matrixc estimate);
      void write_output(gr_complex *out, Matrixc d_matrix);
      Matrixc concatenate(std::vector<Matrixc>& queue);
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

