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
#include <volk/volk.h>

#include <gnuradio/io_signature.h>
#include "frame_generator_bvc_impl.h"

namespace gr {
  namespace fbmc {

    frame_generator_bvc::sptr
    frame_generator_bvc::make(int used_subcarriers, int total_subcarriers, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_bvc_impl(used_subcarriers, total_subcarriers, payload_symbols, payload_bits, overlap, channel_map, preamble));
    }

    /*
     * The private constructor
     */
    frame_generator_bvc_impl::frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers,
                                                       int payload_symbols, int payload_bits, int overlap,
                                                       std::vector<int> channel_map,
                                                       std::vector<gr_complex> preamble) :
            gr::block("frame_generator_bvc", gr::io_signature::make(1, 1, sizeof(char)),
                      gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers)),
            d_used_subcarriers(used_subcarriers), d_total_subcarriers(total_subcarriers),
            d_payload_symbols(payload_symbols), d_payload_bits(payload_bits), d_remaining_payload_bits(payload_bits),
            d_overlap(overlap), /*d_channel_map(channel_map),*/
            d_preamble(preamble), d_frame_position(0)
    {
      setup_preamble(preamble);
      setup_channel_map(channel_map);

      // 2 times overlap because we need 'zero-symbols' after preamble and after payload.
      d_frame_len = d_preamble_symbols + d_payload_symbols + 2 * d_overlap;
      set_output_multiple(2);
    }

    /*
     * Our virtual destructor.
     */
    frame_generator_bvc_impl::~frame_generator_bvc_impl()
    {
      volk_free(d_preamble_buf);
    }

    const gr_complex frame_generator_bvc_impl::D_CONSTELLATION[2] = {gr_complex(-1.0f / std::sqrt(2.0f), 0.0f), gr_complex(1.0f / std::sqrt(2.0f), 0.0f)};

    void
    frame_generator_bvc_impl::setup_preamble(std::vector<gr_complex> preamble)
    {
      if(preamble.size() % d_total_subcarriers != 0) {
        throw std::runtime_error("Parameter mismatch: size(preamble) % total_subcarriers != 0");
      }
      d_preamble_symbols = preamble.size() / d_total_subcarriers;
      d_preamble_buf = (gr_complex*) volk_malloc(sizeof(gr_complex) * d_preamble_symbols * d_total_subcarriers, volk_get_alignment());
      for(int i = 0; i < d_preamble_symbols * d_total_subcarriers; i++){
        d_preamble_buf[i] = preamble[i];
      }
    }

    void
    frame_generator_bvc_impl::setup_channel_map(std::vector<int> channel_map)
    {
      if(channel_map.size() != d_total_subcarriers) {
        throw std::runtime_error("Parameter mismatch: size(channel_map) != total_subcarriers");
      }
      d_channel_map.push_back(std::vector<int>());
      d_channel_map.push_back(std::vector<int>());

      bool inphase = true;
      for(int i = 0; i < d_total_subcarriers; i++){
        if(channel_map[i] == 1){
          if(inphase){
            d_channel_map[0].push_back(i);
          }
          else{
            d_channel_map[1].push_back(i);
          }
        }
        if(i % 2 == 0){
          inphase = !inphase;
        }
      }
    }

    void
    frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      // always only 1 input and 1 output.
      ninput_items_required[0] = noutput_items * (d_used_subcarriers / 4);
    }

    inline void
    frame_generator_bvc_impl::insert_preamble_vector(gr_complex* out, int preamble_position)
    {
      memcpy(out, d_preamble_buf + d_total_subcarriers * preamble_position, sizeof(gr_complex) * d_total_subcarriers);
    }

    inline void
    frame_generator_bvc_impl::insert_padding_zeros(gr_complex* out){
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers);
    }

    inline int
    frame_generator_bvc_impl::insert_payload(gr_complex* out, const char* inbuf)
    {
      const int inphase_sel = inphase_selector();
      const int num_elements = std::min(d_channel_map[inphase_sel].size(), (unsigned long) d_remaining_payload_bits);

      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers);
//      for(int i = 0; i < num_elements; i++){
//        *(out + d_channel_map[inphase_sel][i]) = D_CONSTELLATION[*inbuf++];
//      }

      int i = 0;
      while(i < num_elements){
        *(out + d_channel_map[inphase_sel][i++]) = D_CONSTELLATION[*inbuf++];
      }

      return num_elements;
    }

    int
    frame_generator_bvc_impl::general_work(int noutput_items, gr_vector_int &ninput_items,
                                           gr_vector_const_void_star &input_items,
                                           gr_vector_void_star &output_items)
    {
      const char *in = (const char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      const int nin_items = ninput_items[0];

      int consumed_items = 0;
      int produced_items = 0;
      for(int i = 0; i < noutput_items; i++) {
        if(d_frame_position < d_preamble_symbols) {
          if(d_frame_position == 0)
          {
            d_remaining_payload_bits = d_payload_bits; // current frame's payload is done, reset counter for the next one
          }
          insert_preamble_vector(out, d_frame_position);
        }
        else if(d_frame_position < d_preamble_symbols + d_overlap) {
          insert_padding_zeros(out);
        }
        else if(d_frame_position < d_preamble_symbols + d_overlap + d_payload_symbols) {
          if(nused_items_on_vector() > nin_items - consumed_items){
            break;
          }

          int consumed = insert_payload(out, in);
          d_remaining_payload_bits -= consumed;
          in += consumed;
          consumed_items += consumed;
        }
        else {
          insert_padding_zeros(out);
        }

        d_frame_position = (d_frame_position + 1) % d_frame_len;
        out += d_total_subcarriers;
        ++produced_items;
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each(consumed_items);

      // Tell runtime system how many output items we produced.
      return produced_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

