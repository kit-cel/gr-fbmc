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
#include "tx_dummy_mixer_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    tx_dummy_mixer_cc::sptr
    tx_dummy_mixer_cc::make(int bands, double bandwidth, int symbols, int subcarriers, float samp_rate)
    {
      return gnuradio::get_initial_sptr
        (new tx_dummy_mixer_cc_impl(bands, bandwidth, symbols, subcarriers, samp_rate));
    }

    /*
     * The private constructor
     */
    tx_dummy_mixer_cc_impl::tx_dummy_mixer_cc_impl(int bands, double bandwidth, int symbols, int subcarriers, float samp_rate)
      : gr::block("tx_dummy_mixer_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_bw(bandwidth), d_bands(bands), d_symbols(symbols), d_subcarriers(subcarriers), d_samp_rate(samp_rate)
    {
      set_output_multiple(symbols * subcarriers);
      d_phase_inc.resize(d_bands);
      for (int i = 0; i < d_bands; i++) {
        d_phase_inc[i] = get_freq(i);
        //std::cout << get_freq(i) << " ";
      }
      d_phase = lv_cmake(1, 0);
      d_curr_band = 0;
      d_max = 200;
      d_counter = 0;
    }

    /*
     * Our virtual destructor.
     */
    tx_dummy_mixer_cc_impl::~tx_dummy_mixer_cc_impl()
    {
    }

    void
    tx_dummy_mixer_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_symbols * d_subcarriers;
    }

    gr_complex
    tx_dummy_mixer_cc_impl::get_freq(int band) {
      float arg = 2*M_PI * ((2*band - (d_bands - 1)) * d_bw) / 2.0 / d_samp_rate;
      return lv_cmake(std::cos(arg), std::sin(arg));
      //return std::exp(gr_complex(0, 2*M_PI *((2*band - (d_bands - 1)) * d_bw) / 2.0 / d_samp_rate));
    }

    int
    tx_dummy_mixer_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.

      volk_32fc_s32fc_x2_rotator_32fc(out, in, d_phase_inc[d_curr_band], &d_phase, d_symbols * d_subcarriers);
      //memcpy(out, in, sizeof(gr_complex) * d_symbols * d_subcarriers);
      if(d_counter == d_max) {
        d_curr_band = (d_curr_band + 1) % d_bands;
        d_counter = 0;
      } else {
        d_counter++;
      }
      consume_each (d_symbols * d_subcarriers);
      // Tell runtime system how many output items we produced.
      return d_symbols * d_subcarriers;
    }

  } /* namespace fbmc */
} /* namespace gr */

