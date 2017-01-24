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

#ifndef INCLUDED_FBMC_RX_FREQ_DESPREAD_CVC_IMPL_H
#define INCLUDED_FBMC_RX_FREQ_DESPREAD_CVC_IMPL_H

#include <fbmc/rx_freq_despread_cvc.h>
#include <gnuradio/fft/fft.h>
#include <Eigen/Dense>
#include "helper.h"

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrixf;
typedef Eigen::Matrix<gr_complex, Eigen::Dynamic, Eigen::Dynamic> Matrixc;

namespace gr {
  namespace fbmc {

    class rx_freq_despread_cvc_impl : public rx_freq_despread_cvc
    {
     private:
      int d_subcarriers, d_pilot_timestep, d_o, d_payload_bits, d_frame_len, d_frame_items, d_bands;
      float d_pilot_amplitude;
      std::vector<int> d_pilot_carriers;
      std::vector<float> d_prototype_taps;
      gr::fft::fft_complex* d_fft;
      Matrixf d_G;
      Matrixc d_matrix;
      Matrixf spreading_matrix();
      Matrixc d_channel;
      void write_output(gr_complex* out, int end);
      void channel_estimation(Matrixc R);
      Matrixc equalize(Matrixc R);
      std::vector<gr_complex> matrix_mean(Matrixc matrix, int axis);
      float fine_freq_sync();
      float fine_time_sync();
      helper* d_helper;

     public:
      rx_freq_despread_cvc_impl(std::vector<float> taps, int subcarriers, int bands, int payload_bits, float pilot_amplitude, int pilot_timestep, std::vector<int> pilot_carriers);
      ~rx_freq_despread_cvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_RX_FREQ_DESPREAD_CVC_IMPL_H */

