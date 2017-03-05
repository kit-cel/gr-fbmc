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
#include <numeric>

namespace gr {
  namespace fbmc {

    subchannel_deframer_vcb::sptr
    subchannel_deframer_vcb::make(int subcarriers, int bands, int guard, float threshold,
                                  std::vector<gr_complex> preamble,
                                  int symbols, int bits, std::vector<int> pilot_carriers, int pilot_timestep, int order) {
      return gnuradio::get_initial_sptr
              (new subchannel_deframer_vcb_impl(subcarriers, bands, guard, threshold, preamble, symbols, bits,
                                                pilot_carriers,
                                                pilot_timestep, order));
    }

    /*
     * The private constructor
     */
    subchannel_deframer_vcb_impl::subchannel_deframer_vcb_impl(int subcarriers, int bands, int guard,
                                                               float threshold,
                                                               std::vector<gr_complex> preamble, int symbols, int bits,
                                                               std::vector<int> pilot_carriers, int pilot_timestep, int order)
            : gr::block("subchannel_deframer_vcb",
                        gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * bands),
                        gr::io_signature::make(1, 1, sizeof(char))),
              d_subcarriers(subcarriers), d_bands(bands), d_threshold(threshold), d_symbols(symbols),
              d_preamble(preamble), d_payload_bits(bits),
              d_pilot_carriers(pilot_carriers), d_pilot_timestep(pilot_timestep), d_guard_carriers(guard), d_mod_order(order) {
      d_used_bands.resize(static_cast<unsigned long>(bands));
      set_output_multiple(d_symbols * d_subcarriers * d_bands);
      d_preamble.reserve(static_cast<unsigned long>(d_subcarriers));
      // build vector of used carriers for data
      for (int i = d_guard_carriers; i < d_subcarriers - d_guard_carriers; i++) {
        if (std::find(d_pilot_carriers.begin(), d_pilot_carriers.end(), i) == d_pilot_carriers.end()) {
          d_data_carriers.push_back(i);
        }
      }
    }

    /*
     * Our virtual destructor.
     */
    subchannel_deframer_vcb_impl::~subchannel_deframer_vcb_impl() {
    }

    void
    subchannel_deframer_vcb_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_symbols;
    }

    std::vector<gr_complex>
    subchannel_deframer_vcb_impl::extract_preamble(int band) {

      std::vector<gr_complex> result;
      for (int j = band * d_subcarriers + ((d_guard_carriers+1) & ~1); // this rounds up to the next even integer
           j < band * d_subcarriers + d_subcarriers - ((d_guard_carriers+1) & ~1); j += 2) {
        result.push_back(gr_complex(d_curr_frame[0][j].real(), d_curr_frame[1][j].imag()));
      }

      //for(int i = band * d_subcarriers + d_guard_carriers; i < band * d_subcarriers + d_subcarriers - d_guard_carriers; i += 2) {
      //result.push_back(gr_complex(in[i].real(), in[i+d_subcarriers*d_bands].imag()));
      //std::cout << result.back() << ", ";
      //}
      //for(int i = (band + d_bands -1) * d_subcarriers; i < (band + d_bands -1) * d_subcarriers + d_subcarriers; i += 2) {
      //  result.push_back(in[i]);
      //  std::cout << in[i] << std::endl;
      //}
      return result;
    }

    double
    subchannel_deframer_vcb_impl::correlate(const std::vector<gr_complex> &received) {
      //std::vector<float> abs_square(received.size());
      //volk_32fc_magnitude_squared_32f(&abs_square[0], &received[0], static_cast<unsigned int>(received.size()));
      double energy = 0.0;//std::accumulate(abs_square.begin(), abs_square.end(), 0.0);

      gr_complex correlation[received.size()];
      volk_32fc_x2_multiply_conjugate_32fc(&correlation[0], &d_preamble[(d_guard_carriers + 1) / 2], &received[0],
                                           static_cast<unsigned int>(received.size()));
      //std::cout << d_preamble.size() << ", " << received.size() << std::endl;
      gr_complex corr_result = gr_complex(0, 0);
      for (int i = 0; i < received.size(); i++) {
        corr_result += correlation[i];
        energy += std::sqrt(std::abs(received[i]) * std::abs(received[i]) * std::abs(d_preamble[i+(d_guard_carriers + 1) / 2]) *
                  std::abs(d_preamble[i+(d_guard_carriers + 1) / 2]));
      }
      //std::cout << std::abs(corr_result) << ", " << energy << std::endl;
      return std::abs(corr_result) / energy;
    }

    void
    subchannel_deframer_vcb_impl::detect_used_bands() {
      std::vector<gr_complex> curr_preamble(static_cast<unsigned long>(d_subcarriers));
      for (int b = 0; b < d_bands; b++) {
        curr_preamble = extract_preamble(b);
        d_used_bands[b] = correlate(curr_preamble) >= d_threshold;
        //if(d_used_bands[b]) { std::cout<< "Band " << b << " in use (" << correlate(curr_preamble) << ")" << std::endl; }
      }
    }

    inline char
    subchannel_deframer_vcb_impl::demod(gr_complex sym, int iq) {
      if (iq % 2 != 0) {
        sym = gr_complex(sym.imag(), -sym.real()); // phase rotation
      }
      char symbol;
      //std:: cout << sym.real() << ", ";
      if(d_mod_order == 1) {
      	symbol =  static_cast<char>((sym.real() >= 0.0) ? 1 : 0);
      }
      else if (d_mod_order == 2) {
      	if(sym.real() < -2.0f/std::sqrt(10.0f)) { symbol =  static_cast<char>(0); }
      	else if(sym.real() >= -2.0f/std::sqrt(10.0f) && sym.real() < 0.0) { symbol = static_cast<char>(1); }
      	else if(sym.real() >= 0.0 && sym.real() <= 2.0f/std::sqrt(10.0f)) { symbol = static_cast<char>(2); }
      	else if(sym.real() > 2.0f/std::sqrt(10.0f)) { symbol = static_cast<char>(3); }
      	else {
		      throw std::runtime_error("Unidentified symbol to demodulate");
      	}
      }
      else if (d_mod_order == 4) {
	      if(sym.real() < -6.0f/std::sqrt(42.0f)) { symbol =  static_cast<char>(0); }
      	else if(sym.real() >= -6.0f/std::sqrt(42.0f) && sym.real() < -4.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(1); }
      	else if(sym.real() >= -4.0f / std::sqrt(42.0f) && sym.real() < -2.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(2); }
      	else if(sym.real() >= -2.0/std::sqrt(42.0f) && sym.real() < 0.0) { symbol = static_cast<char>(3); }
        else if(sym.real() >= 0.0 && sym.real() < 2.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(4); }
	      else if(sym.real() >= 2.0f/std::sqrt(42.0f) && sym.real() < 4.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(5); }
	      else if(sym.real() >= 4.0f/std::sqrt(42.0f) && sym.real() < 6.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(6); }
	      else if(sym.real() >= 6.0f/std::sqrt(42.0f)) { symbol = static_cast<char>(7); }
        else {
          throw std::runtime_error("Unidentified symbol to demodulate");
        }
      }
      return symbol;
    }

    void
    subchannel_deframer_vcb_impl::extract_payload(char *out, unsigned int *bits_written) {
      // bands get prcessed highest to lowest
      for (int b = d_bands - 1; b >= 0; b--) {
        if (d_used_bands[b]) {
          // extract data
          for (int k = 2; k < d_symbols; k++) {
            // case: we hit a symbol occupied with pilots or aux pilots
            if ((k - 2) % d_pilot_timestep == 0 || (k - 2) % d_pilot_timestep == 1) {
              for (std::vector<int>::iterator it = d_data_carriers.begin();
                   it != d_data_carriers.end(); ++it) {
                *out++ = demod(d_curr_frame[k][*it + d_subcarriers * b], *it + k);
                (*bits_written) += d_mod_order;
                if(*bits_written % d_payload_bits == 0) { break; }
                //std::cout << k << ", " << *it << ": " << *bits_written << std::endl;
              }
            }
              // case: no pilots in this symbol, extract all carriers
            else {
              for (int n = d_guard_carriers; n < d_subcarriers - d_guard_carriers; n++) {
                *out++ = demod(d_curr_frame[k][n + d_subcarriers * b], n + k);
                (*bits_written) += d_mod_order;
                if(*bits_written % d_payload_bits == 0) { break; }
                //std::cout << k << ", " << n << ": " << *bits_written << std::endl;
              }
            }
          }
        }
      }
    }


    int
    subchannel_deframer_vcb_impl::general_work(int noutput_items,
                                               gr_vector_int &ninput_items,
                                               gr_vector_const_void_star &input_items,
                                               gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      char *out = (char *) output_items[0];
      unsigned int bits_written = 0;
      /*for (int i = 0; i < 2*d_subcarriers; i++) {
        std::cout << in[i] << ", ";
      }
      std::cout << std::endl;*/
      d_curr_frame.clear();
      char *old = out;

      //in += 2*d_subcarriers * d_bands;
      for (int k = 0; k < d_symbols; k++) {
        std::vector<gr_complex> carrier(static_cast<unsigned long>(d_subcarriers * d_bands));
        for (int n = 0; n < d_subcarriers * d_bands; n++) {
          carrier[n] = *in++;
        }
        d_curr_frame.push_back(carrier);
      }
      // std::cout << "Size: " << d_curr_frame.size() << ", " << d_curr_frame[0].size() << std::endl;

      detect_used_bands();
      extract_payload(out, &bits_written);
      /*for (int i = 0; i < bits_written; ++i) {
        std::cout << (int)old[i] << " ";
      }
      if(bits_written > 0) {std::cout << std::endl << std::endl;} */

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each(d_symbols);

      // Tell runtime system how many output items we produced.
      return bits_written/d_mod_order;
    }

  } /* namespace fbmc */
} /* namespace gr */
