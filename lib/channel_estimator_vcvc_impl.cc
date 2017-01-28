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
        channel_estimator_vcvc::make(int subcarriers, std::vector<float> &taps, float pilot_amp, int pilot_timestep,
                                     std::vector<int> &pilot_carriers) {
            return gnuradio::get_initial_sptr
                    (new channel_estimator_vcvc_impl(subcarriers, taps, pilot_amp, pilot_timestep, pilot_carriers));
        }

        /*
         * The private constructor
         */
        channel_estimator_vcvc_impl::channel_estimator_vcvc_impl(int subcarriers, std::vector<float> &taps,
                                                                 float pilot_amp, int pilot_timestep,
                                                                 std::vector<int> &pilot_carriers)
                : gr::sync_block("channel_estimator_vcvc",
                                 gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers),
                                 gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
                  d_subcarriers(subcarriers), d_taps(taps), d_pilot_carriers(pilot_carriers), d_pilot_amp(pilot_amp),
                  d_pilot_timestep(pilot_timestep)
        {
            d_o = (d_taps.size() + 1) / 2;  // overlap factor
            d_missing_symbols = d_pilot_timestep+1;
            set_output_multiple(d_pilot_timestep+1);
            d_current_symbols.resize(d_subcarriers, d_pilot_timestep+1);
        }

        /*
         * Our virtual destructor.
         */
        channel_estimator_vcvc_impl::~channel_estimator_vcvc_impl() {
        }

        int
        channel_estimator_vcvc_impl::work(int noutput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
            const gr_complex *in = (const gr_complex *) input_items[0];
            gr_complex *out = (gr_complex *) output_items[0];

            for(int i = 0; i < std::min(noutput_items, d_missing_symbols); i++) {
                for (int n = 0; n < d_subcarriers; n++) {
                    d_current_symbols(n, i) = in[i * d_subcarriers + n];
                }
                d_missing_symbols--;
            }

            if(d_missing_symbols == 0) {
                //interpolate
                d_missing_symbols = d_pilot_timestep+1;
            }

            // Tell runtime system how many output items we produced.
            return noutput_items;
        }

    } /* namespace fbmc */
} /* namespace gr */

