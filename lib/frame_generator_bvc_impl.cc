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
    frame_generator_bvc::make(int used_subcarriers, int total_subcarriers, int payload_symbols, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_bvc_impl(used_subcarriers, total_subcarriers, payload_symbols, overlap, channel_map, preamble));
    }

    /*
     * The private constructor
     */
    frame_generator_bvc_impl::frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers,
                                                       int payload_symbols, int overlap,
                                                       std::vector<int> channel_map,
                                                       std::vector<gr_complex> preamble) :
            gr::block("frame_generator_bvc", gr::io_signature::make(1, 1, sizeof(char)),
                      gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers)),
            d_used_subcarriers(used_subcarriers), d_total_subcarriers(total_subcarriers),
            d_payload_symbols(payload_symbols), d_overlap(overlap), /*d_channel_map(channel_map),*/
            d_preamble(preamble), d_frame_position(0),
            D_INVSQRT(1.0f / std::sqrt(2.0f))
    {
      setup_preamble(preamble);
      setup_channel_map(channel_map);

      // 2 times overlap because we need 'zero-symbols' after preamble and after payload.
      d_frame_len = d_preamble_symbols + d_payload_symbols + 2 * d_overlap;
    }

    /*
     * Our virtual destructor.
     */
    frame_generator_bvc_impl::~frame_generator_bvc_impl()
    {
      volk_free(d_preamble_buf);
    }

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
      ninput_items_required[0] = noutput_items;
    }

    void
    frame_generator_bvc_impl::insert_preamble_vector(gr_complex* out, int preamble_position)
    {
      memcpy(out, d_preamble_buf + d_total_subcarriers * preamble_position, sizeof(gr_complex) * d_total_subcarriers);
    }

    void
    frame_generator_bvc_impl::insert_padding_zeros(gr_complex* out){
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers);
    }

    int
    frame_generator_bvc_impl::insert_payload(gr_complex* out, const char* inbuf)
    {
      const int inphase_sel = (d_frame_position - d_preamble_symbols + d_overlap) % 2;
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers);
      for(int i = 0; i < d_channel_map[inphase_sel].size(); i++){
        *(out + d_channel_map[inphase_sel][i]) = gr_complex(float(2 * *(inbuf + i) - 1) * D_INVSQRT, 0.0f);
      }
      return d_channel_map[inphase_sel].size();
    }

    int
    frame_generator_bvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // one payload vector will carry d_used_subcarriers / 2 payload symbols!
        const int nin_items = ninput_items[0] - (ninput_items[0] % (d_used_subcarriers / 2));

        int consumed_items = 0;
        for(int i = 0; i < noutput_items; i++){
          if(d_frame_position < d_preamble_symbols){
            insert_preamble_vector(out, d_frame_position);
          }
          else if(d_frame_position < d_preamble_symbols + d_overlap){
            insert_padding_zeros(out);
          }
          else if(d_frame_position < d_preamble_symbols + d_overlap + d_payload_symbols){
            int consumed = insert_payload(out, in);
            in += consumed;
            consumed_items += consumed;
          }
          else{
            insert_padding_zeros(out);
          }

          d_frame_position = (d_frame_position + 1) % d_frame_len;
          out += d_total_subcarriers;
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (consumed_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

