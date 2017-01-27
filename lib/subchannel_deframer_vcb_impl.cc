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

namespace gr {
  namespace fbmc {

    subchannel_deframer_vcb::sptr
    subchannel_deframer_vcb::make(std::vector<gr_complex>& zc_sequence, float min_corr, int fft_len, int num_chan,
                                  int num_symbols, int pilot_timestep, std::vector<int>& pilot_carriers)
    {
      return gnuradio::get_initial_sptr
        (new subchannel_deframer_vcb_impl(zc_sequence, min_corr, fft_len, num_chan, num_symbols, pilot_timestep, pilot_carriers));
    }

    /*
     * The private constructor
     */
    subchannel_deframer_vcb_impl::subchannel_deframer_vcb_impl(std::vector<gr_complex>& zc_sequence, float min_corr,
                                                               int fft_len, int num_chan, int num_symbols,
                                                               int pilot_timestep, std::vector<int>& pilot_carriers)
      : gr::block("subchannel_deframer_vcb",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len),
              gr::io_signature::make(1, 1, sizeof(char))),
        d_num_chan(num_chan), d_zc_sequence(zc_sequence), d_fft_len(fft_len), d_pilot_timestep(pilot_timestep),
        d_pilot_carriers(pilot_carriers), d_min_corr(min_corr), d_num_symbols(num_symbols)
    {
      if(d_fft_len % d_num_chan != 0) {
        throw std::runtime_error("Subcarrier size % number_channels should be 0, but is not!");
      }
      std::vector<std::vector<gr_complex> > size(d_num_symbols, std::vector<gr_complex>(d_fft_len));
      d_current_frame = size;
      std::reverse(d_zc_sequence.begin(), d_zc_sequence.end());
      std::for_each(d_zc_sequence.begin(), d_zc_sequence.end(), [](gr_complex& c) { c = std::conj(c); });

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
      ninput_items_required[0] = d_num_symbols; // process only one frame per work
    }

    int
    subchannel_deframer_vcb_impl::process_one_subband(int band) {
      std::vector<gr_complex> received_preamble;
      for(int k = 0; k < 2; k++) {
        for(int n = band*d_fft_len; k < (band+1)*d_fft_len; k += 2 ) {
          received_preamble.push_back(d_current_frame[k][n]);
        }
      }
      return 0;
    }

    int
    subchannel_deframer_vcb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      char *out = (char *) output_items[0];
      unsigned int bits_written = 0;

      // Do <+signal processing+>
      for(int k = 0; k < d_num_symbols; k++) {
        for(int n = 0; n < d_fft_len; n++) {
          d_current_frame[k][n] = in[k*d_fft_len+n];
        }
      }

      for(int b = 0; b < d_num_chan; b++) {
        bits_written += process_one_subband(b);
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (d_num_symbols); // processed one frame

      // Tell runtime system how many output items we produced.
      return bits_written;
    }

  } /* namespace fbmc */
} /* namespace gr */

