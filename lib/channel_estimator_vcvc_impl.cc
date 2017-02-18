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
#include <numeric>

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
      d_curr_symbol = 0; // frame position
      d_pilot_stored = false; // was there a pilot already?
      d_prev_pilot.resize(d_subcarriers * d_bands * d_o, 1);
      d_curr_pilot.resize(d_subcarriers * d_bands * d_o, 1);
      d_R.resize(d_subcarriers * d_o * d_bands, d_frame_len); //receive data
      // freq positions of pilots
      for(int b = 0; b < d_bands; b++) {
        std::for_each(pilot_carriers.begin(), pilot_carriers.end(), [&](int &c) {
          d_pilot_carriers.push_back(c + b*d_subcarriers);
        });
      }
      d_spread_pilots.resize(d_pilot_carriers.size());
      std::transform(d_pilot_carriers.begin(), d_pilot_carriers.end(), d_spread_pilots.begin(), std::bind1st(std::multiplies<int>(),d_o));
      d_interpolator = new interp2d();
      d_helper = new phase_helper(); // phase unwrap etc.
      set_output_multiple(d_frame_len - std::floor((d_frame_len-2)/d_pilot_timestep) * d_pilot_timestep);
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

    /*std::vector<gr_complex>
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
    }*/

    void
    channel_estimator_vcvc_impl::interpolate_freq(Matrixc estimate) {
      std::vector<int> times {0};
      Matrixc pilots(d_pilot_carriers.size(), 1);

      for (int i = 0; i < d_pilot_carriers.size(); i++) {
        pilots(i, 0) = estimate(d_pilot_carriers[i], 0);
        if(std::abs(pilots(i, 0)) < 0.5) {
        }
      }
      // interpolate in frequency direction
      d_interpolator->set_params(times, d_spread_pilots, pilots);
      for (unsigned int n = 0; n < d_curr_pilot.rows(); n++) {
        d_curr_pilot(n, 0) = d_interpolator->interpolate(0, n);
      }
    }

    void
    channel_estimator_vcvc_impl::interpolate_time(std::vector<Matrixc>& queue) {
      int interpol_span = d_pilot_timestep;
      if(d_curr_symbol == 2) { // first pilot per frame
        interpol_span = d_frame_len - std::floor((d_frame_len-2)/d_pilot_timestep) * d_pilot_timestep;
      }
      std::vector<int> times {0, interpol_span};
      std::vector<int> freqs(d_curr_pilot.size());
      std::iota(freqs.begin(), freqs.end(), 0);
      Matrixc snippet(d_prev_pilot.rows(), d_prev_pilot.cols()+d_curr_pilot.cols());
      snippet << d_prev_pilot, d_curr_pilot;
      Matrixc interp(d_R.rows(), interpol_span);
      d_interpolator->set_params(times, freqs, snippet);
      for (int k = 0; k < interpol_span; k++) {
        for (int n = 0; n < interp.rows(); n++) {
          interp(n, k) = d_interpolator->interpolate(k+1, n);
        }
        d_items_produced++;
      }
      queue.push_back(interp);
    }

    void
    channel_estimator_vcvc_impl::write_output(gr_complex *out, std::vector<Matrixc>& queue) {
      for (int i = 0; i < queue.size(); i++) {
        memcpy(out, queue[i].data(), queue[i].size() * sizeof(gr_complex));
        out += queue[i].size();
      }
    }

    int
    channel_estimator_vcvc_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      d_items_produced = 0;
      // receive matrix
      d_R.resize(d_subcarriers * d_o * d_bands, noutput_items);

      // fill matrix with input data
      memcpy(d_R.data(), in, sizeof(gr_complex) * d_subcarriers * d_o * d_bands * noutput_items);

      Matrixc curr_data(d_subcarriers * d_bands, noutput_items);
      std::vector<Matrixc> queue;
      curr_data = d_G * d_R;

      for (int j = 0; j < curr_data.cols(); j++) {
        if((d_curr_symbol-2) % d_pilot_timestep == 0) { // hit
          interpolate_freq(curr_data.col(j)); // this writes d_curr_pilot
          if(d_pilot_stored) {
            interpolate_time(queue);
          } else { // we have not received other pilots yet - only extrapolation is possible
            for (int i = 0; i < 3; i++) {
              queue.push_back(d_curr_pilot); // 0 order extrapolation in time direction
              d_items_produced++;
            }
          }
          d_prev_pilot = d_curr_pilot;
          d_pilot_stored = true;
        }
        //std::cout << d_curr_symbol << ": " << curr_data(0, j) << std::endl;
        d_curr_symbol++;
        if(d_curr_symbol == d_frame_len) {
          d_curr_symbol = 0;
        }
      }

      write_output(out, queue);
      // Tell runtime system how many output items we produced.
      if(d_curr_symbol < 3) {
        d_curr_symbol = ((d_frame_len-2)/d_pilot_timestep)*d_pilot_timestep + 3;
      } else {
        d_curr_symbol = ((d_curr_symbol - 3) / d_pilot_timestep) * d_pilot_timestep + 3;
      }
      return d_items_produced;
    }

  } /* namespace fbmc */
} /* namespace gr */

