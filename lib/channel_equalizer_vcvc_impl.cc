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
                         gr::io_signature::make(2, 2, sizeof(gr_complex) * subcarriers * overlap * bands),
                         gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands)),
          d_frame_len(frame_len), d_pilot_timestep(pilot_timestep), d_subcarriers(subcarriers),
          d_pilot_amp(pilot_amplitude), d_bands(bands),
          d_pilot_carriers(pilot_carriers), d_taps(taps), d_o(overlap) {
      //set_output_multiple(d_frame_len);
      d_G = spreading_matrix();
      std::cout << d_G.rows() << std::endl;
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

    Matrixf
    channel_equalizer_vcvc_impl::spreading_matrix() {
      Matrixf result(d_subcarriers * d_bands, d_subcarriers * d_o * d_bands);
      // build first row
      for (unsigned int k = 0; k < d_subcarriers * d_o * d_bands; k++) {
        result(0, k) = 0.0;
      }
      for (unsigned int k = 0; k < d_taps.size(); k++) {
        if (k < d_taps.size() / 2) {
          result(0, d_subcarriers * d_o * d_bands - d_taps.size() / 2 + k) = d_taps[k];
        } else {
          result(0, k - d_taps.size() / 2) = d_taps[k];
        }
      }
      int offset = 1;
      for (unsigned int n = 1; n < d_subcarriers * d_bands; n++) {
        for (unsigned int k = 0; k < d_subcarriers * d_o * d_bands; k++) {
          result(n, k) = 0.0;
          if (k >= offset && k < offset + d_taps.size()) {
            result(n, k) = d_taps[k - offset];
          }
        }
        offset += d_o;
      }
      return result;
    }

    void
    channel_equalizer_vcvc_impl::write_output(gr_complex *out, Matrixc data) {
      /*for(unsigned int k = 0; k < data.cols(); k++) {
        for(unsigned int n = 0; n < data.rows(); n++) {
          // TODO phase shift in next block, delete here
          if((k+n) % 2 != 0) {
            out[k * data.rows() + n] = gr_complex(data(n, k).imag(), 0);
          }
          else {
            out[k*data.rows()+n] = gr_complex(data(n, k).real(), 0);
          }
        }
      } */
      for (int i = 0; i < data.size(); i++) {
        out[i] = *(data.data() + i);
      }
    }

    void
    channel_equalizer_vcvc_impl::despread(gr_complex* out, std::vector<gr_complex> R, int noutput_items) {
      gr_complex first[2*d_o-1];
      //std::vector<gr_complex> temp(2*d_o-1);
      for (int k = 0; k < noutput_items; k++) {
        // first symbol
        memcpy(first, &R[(k+1) * d_subcarriers * d_bands * d_o - d_o+1], (d_o-1) * sizeof(gr_complex));
        memcpy(&first[d_o-1], &R[k * d_subcarriers * d_bands * d_o], d_o * sizeof(gr_complex));
        //volk_32fc_32f_multiply_32fc(&temp[0], first, &d_taps[0], 2*d_o-1);
        volk_32fc_32f_dot_prod_32fc(out++, first, &d_taps[0], 2*d_o-1);
       // out[0] = std::accumulate(temp.begin(), temp.end(), gr_complex(0, 0));
        //out++;d_G * d_R
        for(int n = 1; n <= d_subcarriers * d_bands * d_o - 2*d_o+1; n += d_o) {
          //std::cout << k << ", " << n << std::endl;
          //volk_32fc_32f_multiply_32d_G * d_Rfc(&temp[0], &R[n + d_subcarriers * d_bands * d_o * k], &d_taps[0], 2*d_o-1);
          volk_32fc_32f_dot_prod_32fc(out++, &R[n + d_subcarriers * d_bands * d_o * k], &d_taps[0], 2*d_o-1);
          //out[0] = std::accumulate(temp.begin(), temp.end(), gr_complex(0, 0));
          //out++;
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
      //d_data.resize(d_subcarriers * d_bands, noutput_items);
      //memcpy(d_R.data(), in, sizeof(gr_complex) * d_bands * d_subcarriers * d_o * noutput_items);
      volk_32fc_x2_divide_32fc(&d_R[0], in, chan,
                                static_cast<unsigned int>(d_bands * d_subcarriers * d_o * noutput_items)); // zero forcing
      despread(out, d_R, noutput_items);//d_G * d_R; // despreading
      //d_data = d_G * d_R;
      //write_output(out, d_data);
      /*int row = 0;
      for(int i = 0; i < d_data.size(); i++) {
        if(i % (d_subcarriers*d_bands) == 0) {
          std::cout << row << ": ";
        }
        std::cout << out[i] << ", ";
        if((i+1) % (d_subcarriers*d_bands) == 0) {
          std::cout << std::endl;
          row++;
        }
      }
      std::cout << "==================================================" << std::endl;*/

      // Tell runtime system how many output items we produced.

      return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

