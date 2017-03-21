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
#include "sliding_fft_cvc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    sliding_fft_cvc::sptr
    sliding_fft_cvc::make(int subcarriers, int overlap, int bands, int frame_len, bool padding) {
      return gnuradio::get_initial_sptr
          (new sliding_fft_cvc_impl(subcarriers, overlap, bands, frame_len, padding));
    }

    /*
     * The private constructor
     */
    sliding_fft_cvc_impl::sliding_fft_cvc_impl(int subcarriers, int overlap, int bands, int frame_len, bool padding)
        : gr::block("sliding_fft_cvc",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers * overlap * bands)),
          d_subcarriers(subcarriers), d_overlap(overlap), d_bands(bands), d_frame_len(frame_len), d_padding(padding)
    {
      d_fft = new gr::fft::fft_complex(d_subcarriers * d_overlap * d_bands, true);
      //set_output_multiple(d_frame_len);
      if (subcarriers % 2 != 0) {
        throw std::runtime_error("sliding_fft_cvc: Subcarriers not an even number!");
      }
      d_curr_sym = 0;
    }

    /*
     * Our virtual destructor.
     */
    sliding_fft_cvc_impl::~sliding_fft_cvc_impl() {
      delete d_fft;
    }

    void
    sliding_fft_cvc_impl::fftshift(gr_complex *in) {
      // do fftshift
      int fft_len = d_overlap * d_subcarriers * d_bands;
      int tmpbuflen = fft_len / 2;
      gr_complex tmpbuf[tmpbuflen];
      memcpy(tmpbuf, in, sizeof(gr_complex) * (tmpbuflen));
      memcpy(in, &in[tmpbuflen],
             sizeof(gr_complex) * (fft_len - tmpbuflen));
      memcpy(&in[tmpbuflen], tmpbuf,
             sizeof(gr_complex) * (fft_len - tmpbuflen));
    }

    void
    sliding_fft_cvc_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_bands * d_subcarriers * (d_overlap + (noutput_items - 1) / 2);
      //ninput_items_required[0] = d_bands * d_subcarriers * d_overlap;
      //std::cout << "sliding fft forecast [" << ninput_items_required[0] << "->" << noutput_items << "]" << std::endl;
    }

    int
    sliding_fft_cvc_impl::general_work(int noutput_items,
                                       gr_vector_int &ninput_items,
                                       gr_vector_const_void_star &input_items,
                                       gr_vector_void_star &output_items) {

      //std::cout << "============== WORK ============" << std::endl;
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      d_symbol_count = 0;
      d_consume = 0;
      int d_full_symbols = std::min(2 * (ninput_items[0] / (d_bands * d_subcarriers) - d_overlap) + 1, // input items
                                    noutput_items); // choose what is the limiting value

      // Do <+signal processing+>
      //gr_complex fft_result[d_overlap * d_subcarriers * d_bands];
      float normalize = static_cast<float>(1.0 / std::sqrt(d_overlap * d_subcarriers * d_bands));
      for (unsigned int k = 0; k < d_full_symbols; k++) {
        //std::cout << "============== SYMBOL " << k << " ==============" << std::endl;
        //std::cout << "curr sym: " << d_curr_sym << std::endl;
        memcpy(d_fft->get_inbuf(), &in[d_consume],
               d_overlap * d_subcarriers * d_bands * sizeof(gr_complex));
        d_fft->execute();
        //memcpy(fft_result, d_fft->get_outbuf(), d_overlap * d_subcarriers * d_bands * sizeof(gr_complex));
        fftshift(d_fft->get_outbuf());
        volk_32fc_s32fc_multiply_32fc(out, d_fft->get_outbuf(), normalize,
                                      static_cast<unsigned int>(d_overlap * d_subcarriers * d_bands));
        out += d_overlap * d_subcarriers * d_bands;

        d_symbol_count++;
        d_consume += d_bands * d_subcarriers / 2;
        if (d_padding) {
          d_curr_sym++;
          if (d_curr_sym == d_frame_len) {
            d_curr_sym = 0;
            d_consume += d_bands * (d_subcarriers * d_overlap - d_subcarriers / 2);
            //std::cout << "break; " << ++d_frames << std::endl;
            break;
          }
        }
      }
      //std::cout << "symbols processed: " << d_symbol_count << std::endl;
      //std::cout << "samples consumed: " << d_consume << std::endl;
      // Tell runtime system how many input items we consumed on
      // each input stream.

      //std::cout << "consume " << d_consume << std::endl;
      consume_each(d_consume);

      // Tell runtime system how many output items we produced.
      //std::cout << "sliding fft return " << d_symbol_count << std::endl;

      return d_symbol_count;
    }

  } /* namespace fbmc */
} /* namespace gr */
