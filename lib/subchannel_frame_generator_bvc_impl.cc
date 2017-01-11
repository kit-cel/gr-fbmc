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
#include "subchannel_frame_generator_bvc_impl.h"

namespace gr {
  namespace fbmc {

    subchannel_frame_generator_bvc::sptr
    subchannel_frame_generator_bvc::make(int subcarriers, int payload_symbols, int payload_bits, int overlap,
                                         int subchannels, std::vector<float> preamble_symbols, float pilot_amp, int pilot_timestep,
                                         std::vector<int> pilot_carriers)
    {
      return gnuradio::get_initial_sptr
        (new subchannel_frame_generator_bvc_impl(subcarriers, payload_symbols, payload_bits, overlap, subchannels,
                                                 preamble_symbols, pilot_amp, pilot_timestep, pilot_carriers));
    }

    /*
     * The private constructor
     */
    subchannel_frame_generator_bvc_impl::subchannel_frame_generator_bvc_impl(int subcarriers, int payload_symbols,
                                                                             int payload_bits, int overlap,
                                                                             int subchannels,
                                                                             std::vector<float> preamble_symbols,
                                                                             float pilot_amp, int pilot_timestep,
                                                                             std::vector<int> pilot_carriers)
      : gr::block("subchannel_frame_generator_bvc",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
        d_subcarriers(subcarriers), d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_subchannels(subchannels), d_preamble_symbols(preamble_symbols), d_pilot_amp(pilot_amp),
        d_pilot_timestep(pilot_timestep), d_pilot_carriers(pilot_carriers)
    {
      if(d_subcarriers % 2 != 0) {
        throw std::length_error("Subcarriers must be an even number");
      }

      d_frame_len = 2 + d_payload_symbols + 3 * d_overlap;  // why this size?
      set_output_multiple(d_frame_len);
    }

    /*
     * Our virtual destructor.
     */
    subchannel_frame_generator_bvc_impl::~subchannel_frame_generator_bvc_impl()
    {
    }

    const float subchannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {-1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f)};

    void
    subchannel_frame_generator_bvc_impl::insert_preamble(gr_complex *&out)
    {
      float* outptr = (float*) out;
      for(unsigned int i = 0; i < d_preamble_symbols.size(); i++) {
        outptr[2*i] = d_preamble_symbols[i];
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_pilots(gr_complex *&out) {
      float *outptr = (float *) out;
      for (int k = 2; k < d_frame_len; k++) {
        for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
          outptr[k * d_subcarriers + *it] = d_pilot_amp;
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_payload(gr_complex *&out, const char* inbuf)
    {
      float *outptr = (float *) out;
      // build vector of usable carriers for data
      std::vector<int> data_carriers;
      for (int i = 0; i < d_subcarriers; i++) {
        if (std::find(d_pilot_carriers.begin(), d_pilot_carriers.end(), i) == d_pilot_carriers.end()) {
          data_carriers.push_back(i);
        }
      }

      // fill preamble symbols with data in gaps
      for (int k = 0; k < 2; k++) {
        for (int n = 0; n < d_subcarriers/2; n++) {
          outptr[k*d_subcarriers + 2*n+1] = D_CONSTELLATION[*inbuf++];
      }

      // fill remaining symbols with data
      for (int k = 2; k < d_frame_len; k++) {
        // case: we hit a symbol occupied with pilots
        if((k-2) % d_pilot_timestep == 0) {
          for (std::vector<int>::iterator it = data_carriers.begin(); it != data_carriers.end(); ++it) {
            outptr[k * d_subcarriers + *it] = D_CONSTELLATION[*inbuf++];
          }
        }
        // case: no pilots in this symbol, fill all carriers with data
        else {
            for (int n = 0; n < d_subcarriers; n++) {
              outptr[k*d_subcarriers + n] = D_CONSTELLATION[*inbuf++];
            }
          }
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    subchannel_frame_generator_bvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const char *in = (const char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      insert_preamble(out);
      insert_pilots(out);
      insert_payload(out, in);
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (d_payload_bits);

      // Tell runtime system how many output items we produced.
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */

