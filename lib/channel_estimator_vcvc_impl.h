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
#include "helper.h"

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrixf;
typedef Eigen::Matrix<gr_complex, Eigen::Dynamic, Eigen::Dynamic> Matrixc;

namespace gr {
    namespace fbmc {

        class channel_estimator_vcvc_impl : public channel_estimator_vcvc {
        private:
            int d_subcarriers, d_pilot_timestep, d_o, d_frame_len;
            std::vector<float> d_taps;
            std::vector<int> d_pilot_carriers;
            float d_pilot_amp;
            Matrixf d_G;
            Matrixc d_channel;
            Matrixc d_current_symbols;
            helper* d_helper;

            Matrixf spreading_matrix();
            void channel_estimation(Matrixc R);
            Matrixc interpolate_channel();
            void write_output(gr_complex* out, Matrixc d_matrix);

        public:
            channel_estimator_vcvc_impl(int frame_len, int subcarriers, std::vector<float> &taps, float pilot_amp, int pilot_timestep,
                                        std::vector<int> &pilot_carriers);

            ~channel_estimator_vcvc_impl();

            // Where all the action really happens
            int work(int noutput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items);
        };

    } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_IMPL_H */

