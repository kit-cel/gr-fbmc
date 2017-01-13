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
    subchannel_frame_generator_bvc::make(int subcarriers, int payload_bits, int overlap,
                                         std::vector<gr_complex> preamble_symbols, float pilot_amp, int pilot_timestep,
                                         std::vector<int> pilot_carriers)
    {
      return gnuradio::get_initial_sptr
        (new subchannel_frame_generator_bvc_impl(subcarriers, payload_bits, overlap,
                                                 preamble_symbols, pilot_amp, pilot_timestep, pilot_carriers));
    }

    /*
     * The private constructor
     */
    subchannel_frame_generator_bvc_impl::subchannel_frame_generator_bvc_impl(int subcarriers,
                                                                             int payload_bits, int overlap,
                                                                             std::vector<gr_complex> preamble_symbols,
                                                                             float pilot_amp, int pilot_timestep,
                                                                             std::vector<int> pilot_carriers)
      : gr::block("subchannel_frame_generator_bvc",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
        d_subcarriers(subcarriers), d_payload_bits(payload_bits),
        d_overlap(overlap), d_preamble_symbols(preamble_symbols), d_pilot_amp(pilot_amp),
        d_pilot_timestep(pilot_timestep), d_pilot_carriers(pilot_carriers)
    {
      if(d_subcarriers % 2 != 0) {
        throw std::length_error("Subcarriers must be an even number");
      }
      if(d_subcarriers/2 != preamble_symbols.size()) {
        throw std::length_error("Preamble symbols need to be length N/2");
      }
      d_payload_symbols = (int)std::ceil(d_payload_bits / (d_subcarriers - (d_pilot_carriers.size() / d_pilot_timestep)));

      d_frame_len = 2 + d_payload_symbols;  // FIXME why? correct?
      set_output_multiple(d_frame_len);
    }

    const float subchannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {-1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f)};
    const float subchannel_frame_generator_bvc_impl::d_weights_ee[3][7] = {
        {4.29311317e-02f, -1.24972423e-01f, -2.05796767e-01f, 2.39276696e-01f, 2.05796767e-01f, -1.24972423e-01f, -4.29311317e-02f},
        {-6.67270408e-02f, 0.00000000e+00f, 5.64445506e-01f, 0.00000000e+00f, 0.000000000000f, 0.00000000e+00f, -6.67270408e-02f},
        {-4.29311317e-02f, -1.24972423e-01f, 2.05796767e-01f, 2.39276696e-01f, -2.05796767e-01f, -1.24972423e-01f, 4.29311317e-02f}
    };
    const float subchannel_frame_generator_bvc_impl::d_weights_eo[3][7] = {
        {4.29311317e-02f, 1.24972423e-01f, -2.05796767e-01f, -2.39276696e-01f, 2.05796767e-0f, 1.24972423e-01f, -4.29311317e-02f},
        {6.67270408e-02f, 0.00000000e+00f, -5.64445506e-01f, 0.00000000e+00f, 0.000000000000f, 0.00000000e+00f, 6.67270408e-02f},
        {-4.29311317e-02f, 1.24972423e-01f, 2.05796767e-01f, -2.39276696e-01f, -2.05796767e-01f, 1.24972423e-01f, 4.29311317e-02f}
    };
    const float subchannel_frame_generator_bvc_impl::d_weights_oe[3][7] = {
        {-4.29311317e-02f, 1.24972423e-01f, 2.05796767e-01f, -2.39276696e-01f, -2.05796767e-01f, 1.24972423e-01f, 4.29311317e-02f},
        {-6.67270408e-02f, 0.00000000e+00f, 5.64445506e-01f, 0.00000000e+00f, 0.000000000000f, 0.00000000e+00f, -6.67270408e-02f},
        {4.29311317e-02f, 1.24972423e-01f, -2.05796767e-01f, -2.39276696e-01f, 2.05796767e-01f, 1.24972423e-01f, -4.29311317e-02f}
    };
    const float subchannel_frame_generator_bvc_impl::d_weights_oo[3][7] = {
        {-4.29311317e-02f, -1.24972423e-01f, 2.05796767e-01f, 2.39276696e-01f, -2.05796767e-01f, -1.24972423e-01f, 4.29311317e-02f},
        {6.67270408e-02f, 0.00000000e+00f, -5.64445506e-01f, 0.00000000e+00f, 0.000000000000f, 0.00000000e+00f, 6.67270408e-02f},
        {4.29311317e-02f, -1.24972423e-01f, -2.05796767e-01f, 2.39276696e-01f, 2.05796767e-01f, -1.24972423e-01f, -4.29311317e-02f}
    };

    /*
     * Our virtual destructor.
     */
    subchannel_frame_generator_bvc_impl::~subchannel_frame_generator_bvc_impl()
    {
    }

    void
    subchannel_frame_generator_bvc_impl::init_freq_time_frame() {
      std::vector<std::vector<float> > init(d_subcarriers, std::vector<float>(d_frame_len));
      d_freq_time_frame = init;
    }

    void
    subchannel_frame_generator_bvc_impl::write_output(gr_complex*& out) {
      float* outbuf = (float*)out;
      for(unsigned int k = 0; k < d_frame_len; k++) {
        for(unsigned int n = 0; n < d_subcarriers; n++) {
          outbuf[k*d_subcarriers + n] = d_freq_time_frame[n][k];
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_preamble()
    {
      for(unsigned int i = 0; i < d_preamble_symbols.size(); i++) {
        d_freq_time_frame[2*i][0] = d_preamble_symbols[i].real();
        d_freq_time_frame[2*i][1] = d_preamble_symbols[i].imag();
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_pilots() {
      for (unsigned int k = 2; k < d_frame_len - 1; k += d_pilot_timestep) {
        for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
          d_freq_time_frame[*it][k] = d_pilot_amp;
          insert_aux_pilots(*it, k);
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_aux_pilots(const unsigned int N, const unsigned int K) {
      float curr_int = 0.0;  // current interference
      float** weights;
      if(N % 2 == 0) {
        if(K % 2 == 0) {
          weights = (float**)d_weights_ee;
        }
        else {
          weights = (float**)d_weights_eo;
        }
      }
      else {
        if(K % 2 == 0) {
          weights = (float**)d_weights_oe;
        }
        else {
          weights = (float**)d_weights_oo;
        }
      }
      // sum up interference to expect in pilot
      for(unsigned int r = 0; r < 3; r++) {
        for(unsigned int c = 0; c < 7; c++) {
          // check if weight pos is inside the freq/time frame
          if (K-3+c >= 0 && K-3+c < d_frame_len) {
            std::cout << "K=" << K << " N=" << N << " r=" << r << " c=" << c << std::endl;
            curr_int += d_freq_time_frame[(N-1+r+d_subcarriers)%d_subcarriers][K-3+c] * weights[r][c];
          }
        }
      }
      // insert aux pilot p relative to pilot in time domain
      d_freq_time_frame[N][K+1] = -1.0 / weights[1][2] * curr_int;
    }

    void
    subchannel_frame_generator_bvc_impl::insert_payload(const char* inbuf, unsigned int* bits_written)
    {
      // build vector of usable carriers for data
      std::vector<int> data_carriers;
      for (int i = 0; i < d_subcarriers; i++) {
        if (std::find(d_pilot_carriers.begin(), d_pilot_carriers.end(), i) == d_pilot_carriers.end()) {
          data_carriers.push_back(i);
        }
      }

      // fill preamble symbols with data in gaps
      for (int k = 0; k < 2; k++) {
        for (int n = 0; n < (d_subcarriers-1)/2; n++) {
          // fill uneven carriers with data
          d_freq_time_frame[2*n+1][k] = D_CONSTELLATION[*inbuf++];
          (*bits_written)++;
      }

      // fill remaining symbols with data
      for (int k = 2; k < d_frame_len; k++) {
        // case: we hit a symbol occupied with pilots or aux pilots
        if((k-2) % d_pilot_timestep == 0 || (k-2) % d_pilot_timestep == 1) {
          for (std::vector<int>::iterator it = data_carriers.begin(); it != data_carriers.end(); ++it) {
            d_freq_time_frame[*it][k] = D_CONSTELLATION[*inbuf++];
            (*bits_written)++;
            if(*bits_written == d_payload_bits) break;
          }
        }
        // case: no pilots in this symbol, fill all carriers with data
        else {
            for (int n = 0; n < d_subcarriers; n++) {
              d_freq_time_frame[n][k] = D_CONSTELLATION[*inbuf++];
              (*bits_written)++;
              if(*bits_written == d_payload_bits) break;
            }
          }
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_payload_bits;  // FIXME correct forecast
    }

    int
    subchannel_frame_generator_bvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const char *in = (const char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      unsigned int bits_written = 0;
      // clear freq/time matrix
      init_freq_time_frame();
      // Do <+signal processing+>
      insert_payload(in, &bits_written);
      insert_preamble();
      insert_pilots();
      write_output(out);
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (d_payload_bits);

      // Tell runtime system how many output items we produced.
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */
