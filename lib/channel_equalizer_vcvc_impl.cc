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
#include "channel_equalizer_vcvc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    channel_equalizer_vcvc::sptr
    channel_equalizer_vcvc::make(int frame_len, int overlap, int bands, int pilot_timestep,
                                 std::vector<int> pilot_carriers,
                                 int subcarriers, std::vector<float> taps, float pilot_amplitude) {
      return gnuradio::get_initial_sptr
          (new channel_equalizer_vcvc_impl(frame_len, overlap, bands, pilot_timestep, pilot_carriers, subcarriers,
                                           taps, pilot_amplitude));
    }

    /*
     * The private constructor
     */
    channel_equalizer_vcvc_impl::channel_equalizer_vcvc_impl(int frame_len, int overlap, int bands, int pilot_timestep,
                                                             std::vector<int> pilot_carriers, int subcarriers,
                                                             std::vector<float> taps, float pilot_amplitude)
        : gr::sync_block("channel_equalizer_vcvc",
                         gr::io_signature::make2(2, 2, sizeof(gr_complex) * subcarriers * overlap * bands, sizeof(gr_complex) * subcarriers * bands),
                         gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands)),
          d_frame_len(frame_len), d_pilot_timestep(pilot_timestep), d_subcarriers(subcarriers),
          d_pilot_amp(pilot_amplitude), d_bands(bands),
          d_pilot_carriers(pilot_carriers), d_taps(taps), d_o(overlap) {
      //set_output_multiple(d_frame_len);

      /*for (int i = 0; i < d_G.rows(); ++i) {
        for (int j = 0; j < d_G.cols(); ++j) {
          std::cout << d_G(i, j) << " ";
        }
        std::cout << std::endl;
      } */
    }

    /*
     * Our virtual destructor.
     */
    channel_equalizer_vcvc_impl::~channel_equalizer_vcvc_impl() {
    }

    void
    channel_equalizer_vcvc_impl::despread(gr_complex* out, int noutput_items) {
      gr_complex first[2*d_o-1];
      for (int k = 0; k < noutput_items; k++) {
        // first symbol
        memcpy(first, &d_R[(k+1) * d_subcarriers * d_bands * d_o - d_o+1], (d_o-1) * sizeof(gr_complex));
        memcpy(&first[d_o-1], &d_R[k * d_subcarriers * d_bands * d_o], d_o * sizeof(gr_complex));
        volk_32fc_32f_dot_prod_32fc(out++, first, &d_taps[0], 2*d_o-1);
        for(int n = 1; n <= d_subcarriers * d_bands * d_o - 2*d_o+1; n += d_o) {
          volk_32fc_32f_dot_prod_32fc(out, &d_R[n + d_subcarriers * d_bands * d_o * k], &d_taps[0], 2*d_o-1);
          //std::cout << *(out) << " ";
          out++;
        }
      }
    }

    int
    channel_equalizer_vcvc_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      const gr_complex *chan = (const gr_complex *) input_items[1];
      gr_complex *out = (gr_complex *) output_items[0];


      // Do <+signal processing+>

      d_R.resize(d_subcarriers * d_bands * d_o * noutput_items);
      memcpy(&d_R[0], in, sizeof(gr_complex) * d_bands * d_subcarriers * d_o * noutput_items);
      //volk_32fc_x2_divide_32fc(&d_R[0], in, chan,
                               //static_cast<unsigned int>(d_bands * d_subcarriers * d_o * noutput_items)); // zero forcing
      despread(out, noutput_items);//d_G * d_R; // despreading
      volk_32fc_x2_divide_32fc(out, out, chan,
                               static_cast<unsigned int>(d_bands * d_subcarriers * noutput_items)); // zero forcing


      // Tell runtime system how many output items we produced.

      return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */
