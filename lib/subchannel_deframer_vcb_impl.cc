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
#include "subchannel_deframer_vcb_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    subchannel_deframer_vcb::sptr
    subchannel_deframer_vcb::make(int subcarriers, int bands, float threshold, std::vector<gr_complex> preamble,
                                  int symbols, std::vector<int> pilot_carriers, int pilot_timestep)
    {
      return gnuradio::get_initial_sptr
        (new subchannel_deframer_vcb_impl(subcarriers, bands, threshold, preamble, symbols, pilot_carriers,
                                          pilot_timestep));
    }

    /*
     * The private constructor
     */
    subchannel_deframer_vcb_impl::subchannel_deframer_vcb_impl(int subcarriers, int bands, float threshold,
                                                               std::vector<gr_complex> preamble, int symbols,
                                                               std::vector<int> pilot_carriers, int pilot_timestep)
      : gr::block("subchannel_deframer_vcb",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands),
              gr::io_signature::make(1,1, sizeof(char))),
        d_subcarriers(subcarriers), d_bands(bands), d_threshold(threshold), d_preamble(preamble), d_symbols(symbols),
        d_pilot_carriers(pilot_carriers), d_pilot_timestep(pilot_timestep)
    {
      d_used_bands.resize(d_bands);
    }

    /*
     * Our virtual destructor.
     */
    subchannel_deframer_vcb_impl::~subchannel_deframer_vcb_impl()
    {
    }

    void
    subchannel_deframer_vcb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_symbols;
    }

    std::vector<gr_complex>
    subchannel_deframer_vcb_impl::extract_preamble(const gr_complex* in, int band) {
      std::vector<gr_complex> result(d_subcarriers);
      for(int i = band * d_subcarriers; i < band * d_subcarriers + result.size(); i += 2) {
        result.push_back(in[i]);
      }
      for(int i = (band + d_bands -1) * d_subcarriers; i < (band + d_bands -1) * d_subcarriers + result.size(); i += 2) {
        result.push_back(in[i]);
      }
      return result;
    }

    float
    subchannel_deframer_vcb_impl::correlate(std::vector<gr_complex> received) {
      std::vector<float> abs_square(received.size());
      volk_32fc_magnitude_squared_32f(&abs_square[0], &received[0], received.size());
      float energy = std::accumulate(abs_square.begin(), abs_square.end(), 0.0);

      std::vector<gr_complex> correlation(received.size());
      volk_32fc_x2_multiply_conjugate_32fc(&correlation[0], &d_preamble[0], &received[0], received.size());
      float corr_result = std::accumulate(correlation.begin(), correlation.end(), 0.0);
      return corr_result / energy;
    }

    void
    subchannel_deframer_vcb_impl::detect_used_bands(const gr_complex* in) {
      std::vector<gr_complex> curr_preamble(d_subcarriers);
      for(int b = 0; b < d_bands; b++) {
        curr_preamble = extract_preamble(in, b);
        d_used_bands[b] = correlate(curr_preamble) >= d_threshold;
      }
    }


    int
    subchannel_deframer_vcb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      char *out = (char *) output_items[0];
      int bits_written = 0;
      detect_used_bands(in);

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (d_symbols);

      // Tell runtime system how many output items we produced.
      return bits_written;
    }

  } /* namespace fbmc */
} /* namespace gr */

