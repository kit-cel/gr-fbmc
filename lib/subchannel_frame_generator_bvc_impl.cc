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
    subchannel_frame_generator_bvc::make(int subcarriers, int guard_carriers, int payload_bits, int overlap,
                                         std::vector<gr_complex> preamble_symbols, float pilot_amp, int pilot_timestep,
                                         std::vector<int> pilot_carriers, bool padding)
    {
      return gnuradio::get_initial_sptr
        (new subchannel_frame_generator_bvc_impl(subcarriers, guard_carriers, payload_bits, overlap,
                                                 preamble_symbols, pilot_amp, pilot_timestep, pilot_carriers, padding));
    }

    /*
     * The private constructor
     */
    subchannel_frame_generator_bvc_impl::subchannel_frame_generator_bvc_impl(int subcarriers, int guard_carriers,
                                                                             int payload_bits, int overlap,
                                                                             std::vector<gr_complex> preamble_symbols,
                                                                             float pilot_amp, int pilot_timestep,
                                                                             std::vector<int> pilot_carriers,
                                                                             bool padding)
      : gr::block("subchannel_frame_generator_bvc",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
        d_subcarriers(subcarriers), d_payload_bits(payload_bits),
        d_overlap(overlap), d_preamble_symbols(preamble_symbols), d_pilot_amp(pilot_amp),
        d_pilot_timestep(pilot_timestep), d_pilot_carriers(pilot_carriers), d_guard_carriers(guard_carriers),
        d_padding(padding)
    {
      if(d_subcarriers % 2 != 0) {
        throw std::length_error("Subcarriers must be an even number");
      }
      if(d_subcarriers/2 != preamble_symbols.size()) {
        throw std::length_error("Preamble symbols need to be length N/2");
      }
      if(std::find_if(d_pilot_carriers.begin(), d_pilot_carriers.end(), [this](int& carr) {
        return (carr < d_guard_carriers || carr >= d_subcarriers-d_guard_carriers);
      }) != d_pilot_carriers.end()) {
        throw std::length_error("Pilot carriers configured in guard bands!");
      }
      d_payload_symbols = (int)std::ceil((d_payload_bits-d_subcarriers) / (d_subcarriers - (d_pilot_carriers.size() / d_pilot_timestep)));
      // build vector of usable carriers for data
      for (int i = d_guard_carriers; i < d_subcarriers-d_guard_carriers; i++) {
        if (std::find(d_pilot_carriers.begin(), d_pilot_carriers.end(), i) == d_pilot_carriers.end()) {
          d_data_carriers.push_back(i);
        }
      }
      d_frame_len = 2 + d_payload_symbols;
      d_num_zeros = (d_padding) ?  d_overlap * 2 - 1 : 0;
      set_output_multiple(d_frame_len+d_num_zeros);
    }



    const float subchannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {-1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f)}; //std::sqrt(2.0f)
    //const float subchannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {-0.5f , 0.5f }; // better to read in console
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
      std::vector<std::vector<float> > init(static_cast<unsigned long>(d_subcarriers),
                                            std::vector<float>(static_cast<unsigned long>(d_frame_len)));
      d_freq_time_frame = init;
    }

    void
    subchannel_frame_generator_bvc_impl::write_output(gr_complex*& out) {
      for(unsigned int k = 0; k < d_frame_len; k++) {
        for(unsigned int n = 0; n < d_subcarriers; n++) {
          if((k+n) % 2 != 0) {
            out[0] = gr_complex(0, d_freq_time_frame[n][k]);
            out++;
          }
          else {
            out[0] = gr_complex(d_freq_time_frame[n][k], 0);
            out++;
          }
        }
      }
      if(d_padding) {
        memset(out, 0, sizeof(gr_complex)*d_subcarriers * d_num_zeros);
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_preamble()
    {
      for(int i = d_guard_carriers + (d_guard_carriers & 1); i < d_subcarriers - d_guard_carriers; i += 2) {
        d_freq_time_frame[i][0] = d_preamble_symbols[i/2].real();
        d_freq_time_frame[i][1] = d_preamble_symbols[i/2].imag();
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_pilots() {
      for (unsigned int k = 2; k < d_frame_len - 1; k += d_pilot_timestep) {
        for (std::vector<int>::iterator it = d_pilot_carriers.begin(); it != d_pilot_carriers.end(); ++it) {
          d_freq_time_frame[*it][k] = d_pilot_amp;
          insert_aux_pilots(static_cast<unsigned int>(*it), k);
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::insert_aux_pilots(const unsigned int N, const unsigned int K) {
      float curr_int = 0.0;  // current interference
      const float (*weights)[7];  // TODO is this correct?
      if(N % 2 == 0) {
        if(K % 2 == 0) {
          weights = d_weights_ee;
        }
        else {
          weights = d_weights_eo;
        }
      }
      else {
        if(K % 2 == 0) {
          weights = d_weights_oe;
        }
        else {
          weights = d_weights_oo;
        }
      }
      // sum up interference to expect in pilot
      for(unsigned int r = 0; r < 3; r++) {
        for(unsigned int c = 0; c < 7; c++) {
          // check if weight pos is inside the freq/time frame
          if (K-3+c >= 0 && K-3+c < d_frame_len) {
            // add d_subcarriers to ensure positive index when using modulo
            curr_int += d_freq_time_frame[(N-1+r+d_subcarriers)%d_subcarriers][K-3+c] * weights[r][c];
          }
        }
      }
      // insert aux pilot p relative to pilot in time domain
      d_freq_time_frame[N][K+1] = static_cast<float>(-1.0 / weights[1][2] * curr_int);
    }

    void
    subchannel_frame_generator_bvc_impl::insert_payload(const char* inbuf, unsigned int* bits_written)
    {
      /*// fill preamble symbols with data in gaps
      for (int k = 0; k < 2; k++) {
        for (int n = 0; n < d_subcarriers / 2; n++) {
          // fill uneven carriers with data
          d_freq_time_frame[2 * n + 1][k] = D_CONSTELLATION[*inbuf++];
          (*bits_written)++;
        }
      } */

      // fill remaining symbols with data
      for (int k = 2; k < d_frame_len; k++) {
        // case: we hit a symbol occupied with pilots or aux pilots
        if((k-2) % d_pilot_timestep == 0 || (k-2) % d_pilot_timestep == 1) {
          for (std::vector<int>::iterator it = d_data_carriers.begin(); it != d_data_carriers.end(); ++it) {
            d_freq_time_frame[*it][k] = D_CONSTELLATION[*inbuf++];
            (*bits_written)++;
            if(*bits_written == d_payload_bits) break;
          }
        }
        // case: no pilots in this symbol, fill all carriers with data
        else {
            for (int n = d_guard_carriers; n < d_subcarriers-d_guard_carriers; n++) {
              d_freq_time_frame[n][k] = D_CONSTELLATION[*inbuf++];
              (*bits_written)++;
              if(*bits_written == d_payload_bits) break;
            }
        }
      }
    }

    void
    subchannel_frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_payload_bits;
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
      /*int n = 0;
      for(unsigned int i = 0; i < d_frame_len * d_subcarriers; i++) {
        if(n % d_subcarriers == 0 ) { std::cout << n/d_subcarriers << ": "; }
        std::cout << out[i] << ", ";
        n++;
        if(n % d_subcarriers == 0 ) { std::cout << std::endl; }
      }
      std::cout << "=====================================" << std::endl;*/
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (bits_written);
      //std::cout << "subchan_frame_gen: consume " << bits_written << " produce " << d_frame_len << std::endl;
      // Tell runtime system how many output items we produced.
      return d_frame_len + d_num_zeros;
    }

  } /* namespace fbmc */
} /* namespace gr */

