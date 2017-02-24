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
#include <volk/volk.h>

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
                         gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands)),
          d_subcarriers(subcarriers), d_taps(taps), d_pilot_amp(pilot_amp),
          d_pilot_timestep(pilot_timestep), d_frame_len(frame_len), d_o(overlap), d_bands(bands)
    {
      d_curr_symbol = 0; // frame position
      d_pilot_stored = false; // was there a pilot already?
      d_prev_pilot.resize(d_subcarriers * d_bands);
      d_curr_pilot.resize(d_subcarriers * d_bands);
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
      d_base_times.resize(2);
      d_base_freqs.resize(d_subcarriers * d_bands);
      std::iota(d_base_freqs.begin(), d_base_freqs.end(), 0);
      d_snippet.resize(2);
      set_output_multiple(d_frame_len - std::floor((d_frame_len-2)/d_pilot_timestep) * d_pilot_timestep);
    }

    /*
     * Our virtual destructor.
     */
    channel_estimator_vcvc_impl::~channel_estimator_vcvc_impl() {
      delete d_interpolator;
      delete d_helper;
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
    channel_estimator_vcvc_impl::interpolate_freq(std::vector<gr_complex> estimate) {
      std::vector<int> times {0};
      std::vector<gr_complex> pilots(d_pilot_carriers.size());

      for (int i = 0; i < d_pilot_carriers.size(); i++) {
        pilots[i] = estimate[d_pilot_carriers[i]];
      }
      // interpolate in frequency direction
      d_curr_pilot = d_interpolator->interp1d(d_pilot_carriers, d_subcarriers * d_bands, pilots);
    }

    void
    channel_estimator_vcvc_impl::interpolate_time(std::vector<std::vector<gr_complex> >& queue) {
      int interpol_span = d_pilot_timestep;
      if(d_curr_symbol == 2) { // first pilot per frame
        interpol_span = d_frame_len - std::floor((d_frame_len-2)/d_pilot_timestep) * d_pilot_timestep;
      }
      d_base_times[1] = interpol_span;
      d_snippet[0] = d_prev_pilot;
      d_snippet[1] = d_curr_pilot;
      std::vector<std::vector<gr_complex> >* interp = d_interpolator->interpolate(interpol_span, d_subcarriers * d_bands, d_snippet);
      //d_items_produced += interpol_span;
      for (int j = 0; j < (*interp).size(); j++) {
        queue.push_back((*interp)[j]);
      }
    }

    void
    channel_estimator_vcvc_impl::despread(gr_complex* out, int noutput_items) {
      gr_complex first[2*d_o-1];
      for (int k = 0; k < noutput_items; k++) {
        // first symbol
        memcpy(first, &d_R[(k+1) * d_subcarriers * d_bands * d_o - d_o+1], (d_o-1) * sizeof(gr_complex));
        memcpy(&first[d_o-1], &d_R[k * d_subcarriers * d_bands * d_o], d_o * sizeof(gr_complex));
        volk_32fc_32f_dot_prod_32fc(out++, first, &d_taps[0], 2*d_o-1);
        for(int n = 1; n <= d_subcarriers * d_bands * d_o - 2*d_o+1; n += d_o) {
          volk_32fc_32f_dot_prod_32fc(out++, &d_R[n + d_subcarriers * d_bands * d_o * k], &d_taps[0], 2*d_o-1);
        }
      }
    }

    void
    channel_estimator_vcvc_impl::write_output(gr_complex *out, std::vector<std::vector<gr_complex> >& queue) {
      for (int i = 0; i < queue.size(); i++) {
        memcpy(out, &queue[i][0], queue[i].size() * sizeof(gr_complex));
        out += queue[i].size();
        d_items_produced++;
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
      d_R.resize(d_subcarriers * d_o * d_bands * noutput_items);

      // fill matrix with input data
      memcpy(&d_R[0], in, sizeof(gr_complex) * d_subcarriers * d_o * d_bands * noutput_items);

      std::vector<gr_complex> curr_data(d_subcarriers * d_bands * noutput_items);
      std::vector<std::vector<gr_complex> > queue;
      //curr_data = d_G * d_R;
      despread(&curr_data[0], noutput_items);

      for (int j = 0; j < noutput_items; j++) {
        if((d_curr_symbol-2) % d_pilot_timestep == 0) { // hit
          std::vector<gr_complex> temp (&curr_data[d_subcarriers * d_bands * j], &curr_data[d_subcarriers * d_bands * (j+1) - 1]);
          interpolate_freq(temp); // this writes d_curr_pilot
          if(d_pilot_stored) {
            interpolate_time(queue);
            //queue.push_back(d_curr_pilot);
            //d_items_produced++;
          } else { // we have not received other pilots yet - only extrapolation is possible
            for (int i = 0; i < 3; i++) {
              queue.push_back(d_curr_pilot); // 0 order extrapolation in time direction
              //d_items_produced++;
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
