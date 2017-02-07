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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "channel_estimator_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    channel_estimator_vcvc::sptr
    channel_estimator_vcvc::make(int frame_len, int subcarriers, int overlap, int bands, std::vector<float> taps,
                                 float pilot_amp, int pilot_timestep,
                                 std::vector<int> pilot_carriers) {
      return gnuradio::get_initial_sptr
          (new channel_estimator_vcvc_impl(frame_len, subcarriers, overlap, bands, taps, pilot_amp, pilot_timestep,
                                           pilot_carriers));
    }

    /*
     * The private constructor
     */
    channel_estimator_vcvc_impl::channel_estimator_vcvc_impl(int frame_len, int subcarriers, int overlap, int bands,
                                                             std::vector<float> taps,
                                                             float pilot_amp, int pilot_timestep,
                                                             std::vector<int> pilot_carriers)
        : gr::sync_block("channel_estimator_vcvc",
                         gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands * overlap),
                         gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands * overlap)),
          d_subcarriers(subcarriers), d_taps(taps), d_pilot_amp(pilot_amp),
          d_pilot_timestep(pilot_timestep), d_frame_len(frame_len), d_o(overlap), d_bands(bands)
    {
      set_output_multiple(d_frame_len);
      d_R.resize(d_subcarriers * d_o * d_bands, d_frame_len);
      d_G = spreading_matrix();
      for(int b = 0; b < d_bands; b++) {
        std::for_each(pilot_carriers.begin(), pilot_carriers.end(), [&](int &c) {
          d_pilot_carriers.push_back(c + b*d_subcarriers);
        });
      }
      std::vector<int> spread_pilots(d_pilot_carriers.size());
      std::transform(d_pilot_carriers.begin(), d_pilot_carriers.end(), spread_pilots.begin(), std::bind1st(std::multiplies<int>(),d_o));
      d_interpolator = new interp2d(spread_pilots);
      d_helper = new phase_helper();
    }

    /*
     * Our virtual destructor.
     */
    channel_estimator_vcvc_impl::~channel_estimator_vcvc_impl() {
      delete d_interpolator;
      delete d_helper;
    }

    Matrixf
    channel_estimator_vcvc_impl::spreading_matrix() {
      Matrixf result(d_subcarriers *d_bands, d_subcarriers *d_o * d_bands);
      // build first row
      for (unsigned int k = 0; k < d_subcarriers*d_bands*d_o; k++) {
        result(0, k) = 0.0;
      }
      for (unsigned int k = 0; k < d_taps.size(); k++) {
        if (k < d_taps.size() / 2) {
          result(0, d_subcarriers*d_bands*d_o - d_taps.size() / 2 + k) = d_taps[k];
        } else {
          result(0, k - d_taps.size() / 2) = d_taps[k];
        }
      }
      int offset = 1;
      for (unsigned int n = 1; n < d_subcarriers *d_bands; n++) {
        for (unsigned int k = 0; k < d_subcarriers*d_bands*d_o; k++) {
          result(n, k) = 0.0;
          if (k >= offset && k < offset + d_taps.size()) {
            result(n, k) = d_taps[k - offset];
          }
        }
        offset += d_o;
      }
      return result;
    }

    std::vector<gr_complex>
    channel_estimator_vcvc_impl::matrix_mean(Matrixc matrix, int axis) {
      std::vector<gr_complex> result;
      if(axis == 0) { // rowwise
        for(unsigned int k = 0; k < matrix.cols(); k++) {
          gr_complex mean(0, 0);
          for (unsigned int n = 0; n < matrix.rows(); n++) {
            mean += matrix(n, k);
          }
          mean /= gr_complex(matrix.rows(), 0);
          result.push_back(mean);
        }
      }
      else { // columnwise
        for(unsigned int n = 0; n < matrix.rows(); n++) {
          gr_complex mean(0, 0);
          for (unsigned int k = 0; k < matrix.cols(); k++) {
            mean += matrix(n, k);
          }
          mean /= gr_complex(matrix.cols(), 0);
          result.push_back(mean);
        }
      }
      return result;
    }

    double
    channel_estimator_vcvc_impl::fine_freq_sync() {
      std::vector<gr_complex> mean = matrix_mean(d_channel, 0);
      std::vector<double> phase;
      // build phase vector
      for(unsigned int i = 0; i < mean.size(); i++) {
        phase.push_back(d_helper->unwrap(std::arg(mean[i])));
      }
      d_helper->reset_angle();
      double f_o = d_helper->linear_regression(phase)[0];
      f_o /= 2*M_PI*d_subcarriers;
      return f_o;
    }

    double
    channel_estimator_vcvc_impl::fine_time_sync() {
      std::vector<gr_complex> mean = matrix_mean(d_channel, 1);
      std::vector<double> phase;
      // build phase vector
      for(unsigned int i = 0; i < mean.size(); i++) {
        phase.push_back(d_helper->unwrap(std::arg(mean[i])));
      }
      d_helper->reset_angle();
      double t_o = d_helper->linear_regression(phase)[0];
      t_o /= 2*M_PI;
      return t_o;
    }

    void
    channel_estimator_vcvc_impl::channel_estimation(Matrixc R) {
      gr_complex pilot_sym;
      long K = (R.cols() - 2) / d_pilot_timestep + 1; // number of symbols containing pilots
      Matrixc estimate(d_pilot_carriers.size(), K);
      for (unsigned int k = 0; k < K; k++) {
        int i = 0;
        for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
          if((k*d_pilot_timestep+*it) % 2 == 0) {
            pilot_sym = R(*it, k * d_pilot_timestep + 2);
          } else {
            pilot_sym = gr_complex(R(*it, k * d_pilot_timestep + 2).imag(), -R(*it, k * d_pilot_timestep + 2).real());
          }
          estimate(i, k) = pilot_sym / d_pilot_amp;  // channel estimation
          //std::cout << estimate(i, k) << ", ";
          i++;
        }
        //std::cout << std::endl;
      }
      d_channel = estimate;
    }

    Matrixc
    channel_estimator_vcvc_impl::interpolate_channel() {
      // vector of symbol indexes with pilots
      std::vector<int> pilot_times;
      Matrixc R_eq(d_R.rows(), d_R.cols());
      for (int i = 2; i < d_R.cols(); i += d_pilot_timestep) {
        pilot_times.push_back(i);
      }
      d_interpolator->set_params(pilot_times, d_channel);
      for (unsigned int k = 0; k < d_R.cols(); k++) {
        for (unsigned int n = 0; n < d_R.rows(); n++) {
          R_eq(n, k) = d_interpolator->interpolate(k, n);
          //std::cout << R_eq(n, k) << ", ";
        }
        //std::cout << std::endl << std::endl;
      }
      return R_eq;
    }

    void
    channel_estimator_vcvc_impl::write_output(gr_complex *out, Matrixc d_matrix) {
      for (int i = 0; i < d_matrix.size(); i++) {
        out[i] = *(d_matrix.data() + i);
      }
    }

    int
    channel_estimator_vcvc_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for (int i = 0; i < d_R.size(); i++) {
        *(d_R.data() + i) = in[i];
      }

      Matrixc curr_data(d_subcarriers * d_bands, d_frame_len);
      Matrixc result(d_subcarriers * d_bands * d_o, d_frame_len);
      curr_data = d_G * d_R;
      // estimate channel with pilots
      channel_estimation(curr_data);
      result = interpolate_channel();
      write_output(out, result);
      // Tell runtime system how many output items we produced.
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */

