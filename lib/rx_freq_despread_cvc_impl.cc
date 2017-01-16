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
#include "rx_freq_despread_cvc_impl.h"

namespace gr {
  namespace fbmc {

    rx_freq_despread_cvc::sptr
    rx_freq_despread_cvc::make(std::vector<float> taps, int subcarriers, float pilot_amplitude, int pilot_timestep, std::vector<int> pilot_carriers)
    {
      return gnuradio::get_initial_sptr
        (new rx_freq_despread_cvc_impl(taps, subcarriers, pilot_amplitude, pilot_timestep, pilot_carriers));
    }

    /*
     * The private constructor
     */
    rx_freq_despread_cvc_impl::rx_freq_despread_cvc_impl(std::vector<float> taps, int subcarriers, float pilot_amplitude, int pilot_timestep, std::vector<int> pilot_carriers)
      : gr::block("rx_freq_despread_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
        d_prototype_taps(taps), d_subcarriers(subcarriers), d_pilot_amplitude(pilot_amplitude),
        d_pilot_timestep(pilot_timestep), d_pilot_carriers(pilot_carriers)
    {
      d_o = (d_prototype_taps.size() + 1) / 2;  // overlap factor
      d_fft = new gr::fft::fft_complex(d_subcarriers * d_o, true);
      d_G = spreading_matrix();
    }

    /*
     * Our virtual destructor.
     */
    rx_freq_despread_cvc_impl::~rx_freq_despread_cvc_impl()
    {
      delete d_fft;
    }

    void
    rx_freq_despread_cvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = (noutput_items - 1) * d_subcarriers/2 + d_subcarriers*d_o;
    }

    Matrixf
    rx_freq_despread_cvc_impl::spreading_matrix() {
      Matrixf result(d_subcarriers, d_subcarriers * d_o);
      // build first row
      for(unsigned int k = 0; k < d_subcarriers * d_o; k++) {
        result(0, k) = 0.0;
      }
      for(unsigned int k = 0; k < d_prototype_taps.size(); k++) {
        if(k < d_prototype_taps.size()/2) {
          result(0, d_subcarriers * d_o - d_prototype_taps.size()/2 + k) = d_prototype_taps[k];
        }
        else {
          result(0, k - d_prototype_taps.size()/2) = d_prototype_taps[k];
        }
      }
      int offset = 1;
      for(unsigned int n = 1; n < d_subcarriers; n++) {
        for(unsigned int k = 0; k < d_subcarriers * d_o; k++) {
          result(n, k) = 0.0;
          if (k >= offset && k <= offset+d_prototype_taps.size()) {
            result(n, k) = d_prototype_taps[k-offset];
          }
        }
        offset += d_o;
      }
      return result;
    }

    void
    rx_freq_despread_cvc_impl::write_output(gr_complex* out, Matrixc in) {
      for(unsigned int k = 0; k < in.cols(); k++) {
        for(unsigned int n = 0; n < in.rows(); n++) {
          out[k*in.rows()+n] = in(n, k);
        }
      }
    }

    void
    rx_freq_despread_cvc_impl::channel_estimation(Matrixc R) {
      unsigned int K = (R.cols()-2)/d_pilot_timestep + 1;
      Matrixc estimate(d_pilot_carriers.size(), K);

      for(unsigned int k = 0; k < K; k++) {
        int i = 0;
        for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
          estimate(i, k) = R(*it, k * d_pilot_timestep + 2) / d_pilot_amplitude;  // channel estimation
          i++;
        }
      }
      d_channel = estimate;
    }

    int
    rx_freq_despread_cvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      int num_symbols = 2*(ninput_items[0]-1)/d_subcarriers;
      Matrixc R(d_o * d_subcarriers, num_symbols);
      gr_complex fft_result[d_o*d_subcarriers];
      for(unsigned int k = 0; k <= num_symbols; k++) {
        for(unsigned int n = 0; n < d_o * d_subcarriers; n++) {
          memcpy(d_fft->get_inbuf(), &in[k*d_subcarriers/2], d_o * d_subcarriers);
          d_fft->execute();
          memcpy(fft_result, d_fft->get_outbuf(), d_o*d_subcarriers);
          R(n, k) = fft_result[n];
        }
      }

      Matrixc d_matrix(d_subcarriers, num_symbols);
      d_matrix = d_G * R;
      channel_estimation(d_matrix);
      write_output(out, d_matrix);
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (d_subcarriers * num_symbols);

      // Tell runtime system how many output items we produced.
      return num_symbols;
    }

  } /* namespace fbmc */
} /* namespace gr */

