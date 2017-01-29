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
        channel_estimator_vcvc::make(int frame_len, int subcarriers, std::vector<float> &taps, float pilot_amp, int pilot_timestep,
                                     std::vector<int> &pilot_carriers) {
            return gnuradio::get_initial_sptr
                    (new channel_estimator_vcvc_impl(frame_len, subcarriers, taps, pilot_amp, pilot_timestep, pilot_carriers));
        }

        /*
         * The private constructor
         */
        channel_estimator_vcvc_impl::channel_estimator_vcvc_impl(int frame_len, int subcarriers, std::vector<float> &taps,
                                                                 float pilot_amp, int pilot_timestep,
                                                                 std::vector<int> &pilot_carriers)
                : gr::sync_block("channel_estimator_vcvc",
                                 gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers),
                                 gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
                  d_subcarriers(subcarriers), d_taps(taps), d_pilot_carriers(pilot_carriers), d_pilot_amp(pilot_amp),
                  d_pilot_timestep(pilot_timestep), d_frame_len(frame_len)
        {
            d_o = (d_taps.size() + 1) / 2;  // overlap factors
            set_output_multiple(d_frame_len);
            d_current_symbols.resize(d_subcarriers, d_frame_len);
            d_G = spreading_matrix();
            d_helper = new helper(d_pilot_carriers);
        }

        /*
         * Our virtual destructor.
         */
        channel_estimator_vcvc_impl::~channel_estimator_vcvc_impl() {
            delete d_helper;
        }

        Matrixf
        channel_estimator_vcvc_impl::spreading_matrix() {
            Matrixf result(d_subcarriers/d_o, d_subcarriers);
            // build first row
            for(unsigned int k = 0; k < d_subcarriers; k++) {
                result(0, k) = 0.0;
            }
            for(unsigned int k = 0; k < d_taps.size(); k++) {
                if(k < d_taps.size()/2) {
                    result(0, d_subcarriers - d_taps.size()/2 + k) = d_taps[k];
                }
                else {
                    result(0, k - d_taps.size()/2) = d_taps[k];
                }
            }
            int offset = 1;
            for(unsigned int n = 1; n < d_subcarriers/d_o; n++) {
                for(unsigned int k = 0; k < d_subcarriers; k++) {
                    result(n, k) = 0.0;
                    if (k >= offset && k < offset+d_taps.size()) {
                        result(n, k) = d_taps[k-offset];
                    }
                }
                offset += d_o;
            }
            return result;
        }

        void
        channel_estimator_vcvc_impl::channel_estimation(Matrixc R) {
            unsigned int K = (R.cols()-2)/d_pilot_timestep + 1; // number of symbols containing pilots
            Matrixc estimate(d_pilot_carriers.size(), K);
            for(unsigned int k = 0; k < K; k++) {
                int i = 0;
                for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
                    estimate(i, k) = R(*it, k * d_pilot_timestep + 2) / d_pilot_amp;  // channel estimation
                    i++;
                }
            }
            d_channel = estimate;
        }

        Matrixc
        channel_estimator_vcvc_impl::interpolate_channel() {
            // vector of symbol indexes with pilots
            std::vector<int> pilot_times;
            Matrixc R_eq(d_current_symbols.rows(), d_current_symbols.cols());
            for(unsigned int i = 2; i < d_current_symbols.cols(); i += d_pilot_timestep) {
                pilot_times.push_back(i);
            }
            d_helper->set_params(pilot_times, d_channel);
            for(unsigned int k = 0; k < d_current_symbols.cols(); k++) {
                for(unsigned int n = 0; n < d_current_symbols.rows(); n++) {
                    R_eq(n, k) = d_helper->get_value(k, n);
                }
            }
            return R_eq;
        }

        void
        channel_estimator_vcvc_impl::write_output(gr_complex* out, Matrixc d_matrix) {
            for(unsigned int k = 0; k < d_matrix.cols(); k++) {
                for(unsigned int n = 0; n < d_matrix.rows(); n++) {
                    if(k*d_matrix.rows()+n >= d_frame_len * d_subcarriers) { break ;}
                    // TODO phase shift in next block, delete here
                    if((k+n) % 2 != 0) {
                        out[k * d_matrix.rows() + n] = gr_complex(d_matrix(n, k).imag(), -d_matrix(n, k).real());
                    }
                    else {
                        out[k*d_matrix.rows()+n] = d_matrix(n, k);
                    }
                }
            }
        }

        int
        channel_estimator_vcvc_impl::work(int noutput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
            const gr_complex *in = (const gr_complex *) input_items[0];
            gr_complex *out = (gr_complex *) output_items[0];

            for(int i = 0; i < d_frame_len; i++) {
                for (int n = 0; n < d_subcarriers; n++) {
                    d_current_symbols(n, i) = in[i * d_subcarriers + n];
                }
            }

            Matrixc curr_data(d_subcarriers/d_o, d_frame_len);
            Matrixc result(d_subcarriers, d_frame_len);
            curr_data = d_G * d_current_symbols;
            // estimate channel with pilots
            channel_estimation(curr_data);
            result = interpolate_channel();
            write_output(out, result);
            // Tell runtime system how many output items we produced.
            return d_frame_len;
        }

    } /* namespace fbmc */
} /* namespace gr */

