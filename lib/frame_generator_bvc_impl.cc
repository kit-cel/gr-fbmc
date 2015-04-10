/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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

#include <stdexcept>

#include <gnuradio/io_signature.h>
#include "frame_generator_bvc_impl.h"

namespace gr {
  namespace fbmc {

    frame_generator_bvc::sptr
    frame_generator_bvc::make(int used_subcarriers, int total_subcarriers, int payload_symbols, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_bvc_impl(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble));
    }

    /*
     * The private constructor
     */
    frame_generator_bvc_impl::frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers, int payload_symbols, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
      : gr::block("frame_generator_bvc",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers)),
              d_used_subcarriers(used_subcarriers),
              d_total_subcarriers(total_subcarriers),
              d_payload_symbols(payload_symbols),
              d_overlap(overlap),
              d_channel_map(channel_map),
              d_preamble(preamble),
              d_frame_position(0)
    {
      if(channel_map.size() != total_subcarriers){
        throw std::runtime_error("Parameter mismatch: size(channel_map) != total_subcarriers");
      }

      if(preamble.size() % total_subcarriers != 0){
        throw std::runtime_error("Parameter mismatch: size(preamble) % total_subcarriers != 0");
      }
      d_preamble_symbols = preamble.size() / total_subcarriers;

      // 2 times overlap because we need 'zero-symbols' after preamble and after payload.
      d_frame_len = d_preamble_symbols + d_payload_symbols + 2 * d_overlap;
    }

    /*
     * Our virtual destructor.
     */
    frame_generator_bvc_impl::~frame_generator_bvc_impl()
    {
    }

    void
    frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    void
    frame_generator_bvc_impl::insert_preamble_vector(gr_complex* out, int preamble_position)
    {
      gr_complex* preample_ptr = &d_preamble[d_total_subcarriers * preamble_position];
      memcpy(out, preample_ptr, sizeof(gr_complex) * d_total_subcarriers);
    }

    int
    frame_generator_bvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        const int nin_items = ninput_items[0];

        for(int i = 0; i < noutput_items; i++){
          if(d_frame_position < d_preamble_symbols){
            insert_preamble_vector(out, d_frame_position);
          }

//          if()

          d_preamble_symbols = (d_preamble_symbols + 1) % d_frame_len;
        }


        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

