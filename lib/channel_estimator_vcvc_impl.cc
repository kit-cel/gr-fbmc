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
#include <algorithm>
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
                         gr::io_signature::make(2, 2, sizeof(gr_complex) * subcarriers * bands)),
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
      d_pilots.resize(d_pilot_carriers.size());
      // only used when equalizing in spread domain
      //d_spread_pilots.resize(d_pilot_carriers.size());
      //std::transform(d_pilot_carriers.begin(), d_pilot_carriers.end(), d_spread_pilots.begin(), std::bind1st(std::multiplies<int>(),d_o));
      d_interpolator = new interp2d(d_subcarriers * d_bands);

      // needed for fine freq/timing correction - not used right now
      //d_helper = new phase_helper(); // phase unwrap etc.
      d_base_times.resize(2); // time index of pilots
      d_base_freqs.resize(d_subcarriers * d_bands); // frequency index of pilots
      d_curr_data.resize(d_subcarriers * d_bands * 100); // despread data
      std::iota(d_base_freqs.begin(), d_base_freqs.end(), 0); // used for timing interpolation (= no freq interpolation)
      d_snippet.resize(2);
      // maximum timestep that could occur with current settings. Wrong config leads to deadlock
      set_output_multiple(std::max(
          static_cast<int>(d_frame_len - std::floor((d_frame_len-3)/d_pilot_timestep) * d_pilot_timestep),
                                   d_pilot_timestep));
      set_max_noutput_items(100);
      d_frame_counter = 0;
    }

    /*
     * Our virtual destructor.
     */
    channel_estimator_vcvc_impl::~channel_estimator_vcvc_impl() {
      delete d_interpolator;
      //delete d_helper;
    }

    // the following is fine frequency and timing estimation functionality and not used for dyspan

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
    channel_estimator_vcvc_impl::interpolate_freq(std::vector<gr_complex>::iterator estimate) {
      //std::cout << "Call to work Chan Est" << std::endl;
      for (int i = 0; i < d_pilot_carriers.size(); i++) {
        if((d_curr_symbol + d_pilot_carriers[i]) % 2 == 0) {
          d_pilots[i] = *(estimate + d_pilot_carriers[i]) / gr_complex(d_pilot_amp, 0);
          //if(d_curr_symbol == 16) {
          //  std::cout << d_curr_symbol << "," << d_pilot_carriers[i] << ": [Even] " << d_pilots[i] << std::endl;
          //}
        } else {
          d_pilots[i] = gr_complex((*(estimate + d_pilot_carriers[i])).imag(),
                                   -(*(estimate + d_pilot_carriers[i])).real()) / gr_complex(d_pilot_amp, 0);
          //if(std::abs(d_pilots[i].imag()) > 0.8 && std::abs(d_pilots[i].real()) < 0.1) {
          //  std::cout << d_curr_symbol << "," << d_pilot_carriers[i] << ":[Odd]  " << d_pilots[i] << std::endl;
          //}
        }
        //std::cout << d_curr_symbol << ": " << *(estimate + d_pilot_carriers[i]) << std::endl;
        /*if(d_curr_symbol == 2) {
          for (int j = 0; j < d_subcarriers * d_bands; ++j) {
            std::cout << j << ":" << *(estimate + j) << std::endl;
          }
        }*/
      }

      // interpolate in frequency direction
      d_curr_pilot = d_interpolator->interp1d(d_pilot_carriers, d_subcarriers * d_bands, d_pilots);
    }

    void
    channel_estimator_vcvc_impl::interpolate_time(gr_complex* out) {
      int interpol_span = d_pilot_timestep;
      if(d_curr_symbol == 2) { // first pilot per frame
        int lastpilot = 2;
        while(lastpilot < d_frame_len) {
          lastpilot += d_pilot_timestep;
        }
        lastpilot -= d_pilot_timestep;
        interpol_span = d_frame_len - lastpilot + 2;
      }
      //std::cout << "Interpolating " << interpol_span << std::endl;
      // first base time is always 0
      d_base_times[1] = interpol_span;
      // data to interpolate in between
      d_snippet[0] = d_prev_pilot;
      d_snippet[1] = d_curr_pilot;
      d_items_produced += d_interpolator->interpolate(out, interpol_span, d_subcarriers * d_bands, d_snippet);
    }

    inline void
    channel_estimator_vcvc_impl::despread(gr_complex* out, const gr_complex* in, int noutput_items) {
      gr_complex first[2*d_o-1];
      for (int k = 0; k < noutput_items; k++) {
        // first symbol - special case
        memcpy(first, &in[(k+1) * d_subcarriers * d_bands * d_o - d_o+1], (d_o-1) * sizeof(gr_complex));
        memcpy(&first[d_o-1], &in[k * d_subcarriers * d_bands * d_o], d_o * sizeof(gr_complex));
        volk_32fc_32f_dot_prod_32fc(out++, first, d_taps.data(), 2*d_o-1);
        // all other symbols
        for(int n = 1; n <= d_subcarriers * d_bands * d_o - 2*d_o+1; n += d_o) {
          volk_32fc_32f_dot_prod_32fc(out++, &in[n + d_subcarriers * d_bands * d_o * k], d_taps.data(), 2*d_o-1);
        }
      }
    }

    int
    channel_estimator_vcvc_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[1];
      gr_complex *data = (gr_complex *) output_items[0];

      d_items_produced = 0;  // item counter for current work
      d_curr_data.clear(); // dump previous data
      d_curr_data.resize(noutput_items);

      despread(&d_curr_data[0], in, noutput_items); // frequency despreading
      //std::cout << "Call to work" << std::endl;
      int tempsymbol = d_curr_symbol;
      for (int k = 0; k < noutput_items; ++k) {
        //if((tempsymbol-2) % d_pilot_timestep == 0) {
          for (int n = 0; n < d_pilot_carriers.size(); ++n) {
            //if (std::abs(d_curr_data[k * d_subcarriers * d_bands + n].real() - d_pilot_amp) > 0.1) {
             // std::cout << "[" << d_frame_counter << "] " << "Pilot at " << tempsymbol << ", " << d_pilot_carriers[n] << ": "
             //           << d_curr_data[k * d_subcarriers * d_bands + d_pilot_carriers[n]] << std::endl;
            //}
         // }
        }
        tempsymbol++;
        if(tempsymbol == d_frame_len) {
          tempsymbol = 0;
        }
      }

      // logic to extract pilot symbols
      for (int j = 0; j < noutput_items; j++) {
        if((d_curr_symbol-2) % d_pilot_timestep == 0) { // hit
          // frequency interpolation over one symbol
          interpolate_freq(d_curr_data.begin() + (d_subcarriers * d_bands * j)); // this writes d_curr_pilot
          if(d_pilot_stored) { // case we have received another pilot symbol to interpolate in time
            interpolate_time(out); // linear time interpolation
            // the following implements zero order interpolation
            //memcpy(out, &d_curr_pilot[0], d_subcarriers * d_bands * sizeof(gr_complex));
            //out += d_subcarriers * d_bands;
            //d_items_produced++;
          } else { // we have not received other pilots yet - only extrapolation is possible
            for (int i = 0; i < 3; i++) {
              memcpy(out, &d_curr_pilot[0], d_subcarriers * d_bands * sizeof(gr_complex));
              out += d_subcarriers * d_bands;
              d_items_produced++;
            }
          }
          d_prev_pilot = d_curr_pilot; // set current pilot as previous pilot for next work()
          d_pilot_stored = true;
        }
        d_curr_symbol++; // in-frame symbol counter
        if(d_curr_symbol == d_frame_len) {
          d_curr_symbol = 0; // counter reset at frame end
          d_frame_counter++;
        }
      }

      //std::cout << "d_curr_symbol: " <<d_curr_symbol << std::endl;

      // logic to reset current symbol to effectively processed symbols (we may have counted more)
      if(d_curr_symbol < 3) {
        d_curr_symbol = (((d_frame_len-3)/d_pilot_timestep)*d_pilot_timestep + 3)%d_frame_len;
      } else {
        d_curr_symbol = (((d_curr_symbol - 3) / d_pilot_timestep) * d_pilot_timestep + 3)%d_frame_len;
      }
      //std::cout << "d_curr_symbol (reset): " <<d_curr_symbol << std::endl;

      // copy despread data into output buffer
      memcpy(data, d_curr_data.data(), sizeof(gr_complex) * d_subcarriers * d_bands * d_items_produced);
      for (int k = 0; k < d_items_produced * d_subcarriers * d_bands; ++k) {
        //std::cout << out[k] << ", ";
      }
      //std::cout << "returning " << d_items_produced << std::endl;
      return d_items_produced;
    }

  } /* namespace fbmc */
} /* namespace gr */
