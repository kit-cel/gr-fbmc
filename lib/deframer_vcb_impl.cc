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

#include <gnuradio/io_signature.h>
#include "deframer_vcb_impl.h"

namespace gr {
  namespace fbmc {

    deframer_vcb::sptr
    deframer_vcb::make(int used_subcarriers, int total_subcarriers, int num_preamble_symbols, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map)
    {
      return gnuradio::get_initial_sptr
        (new deframer_vcb_impl(used_subcarriers, total_subcarriers, num_preamble_symbols, payload_symbols, payload_bits, overlap, channel_map));
    }

    /*
     * The private constructor
     */
    deframer_vcb_impl::deframer_vcb_impl(int used_subcarriers, int total_subcarriers, int num_preamble_symbols,
                                         int payload_symbols, int payload_bits, int overlap,
                                         std::vector<int> channel_map) :
            gr::block("deframer_vcb",
                      gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers),
                      gr::io_signature::make(1, 1, sizeof(char))),
            d_used_subcarriers(used_subcarriers), d_total_subcarriers(total_subcarriers),
            d_payload_symbols(payload_symbols), d_payload_bits(payload_bits), d_remaining_payload_bits(payload_bits),
            d_overlap(overlap), d_preamble_symbols(num_preamble_symbols), d_frame_position(0)
    {
      setup_channel_map(channel_map);
      d_frame_len = d_preamble_symbols + d_overlap + d_payload_symbols + d_overlap;
      d_start_of_payload = d_preamble_symbols + d_overlap;
      d_end_of_payload = d_preamble_symbols + d_overlap + d_payload_symbols;

      // this should prevent work from producing more items than the out buffer can hold.
      set_output_multiple(d_used_subcarriers);
    }

    /*
     * Our virtual destructor.
     */
    deframer_vcb_impl::~deframer_vcb_impl()
    {
    }

    void
    deframer_vcb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      // this block "interpolates" because it takes vectors and puts out a stream.
      // the number is an approximation. take into account that every frame has overlap and preamble symbols which are dropped here.
      ninput_items_required[0] = noutput_items / (d_used_subcarriers / 2);
    }

    void
    deframer_vcb_impl::setup_channel_map(std::vector<int> channel_map)
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

    int
    deframer_vcb_impl::extract_bits(char* out, const gr_complex* inbuf)
    {
      const int inphase_sel = inphase_selector();
      int num_elements = std::min(d_channel_map[inphase_sel].size(), (unsigned long) d_remaining_payload_bits);
      for(int i = 0; i < num_elements; i++){
        if((*(inbuf + d_channel_map[inphase_sel][i])).real() > 0.0f){
          *(out + i) = 1;
        }
        else{
          *(out + i) = 0;
        }
      }
      return d_channel_map[inphase_sel].size();
    }

    int
    deframer_vcb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        char *out = (char *) output_items[0];

        const int nin_items = ninput_items[0];

        int produced_items = 0;
        int consumed_items = 0;
        for(int items = 0; items < nin_items; items++){
          if(d_start_of_payload <= d_frame_position && d_frame_position < d_end_of_payload){
            if(nused_items_on_vector() > noutput_items - produced_items){
              break;
            }
            int produced = extract_bits(out, in);
            d_remaining_payload_bits -= produced;
            out += produced;
            produced_items += produced;
          }

          d_frame_position = (d_frame_position + 1) % d_frame_len;
          if(d_frame_position == 0) // reset remaining payload bits for the new frame
          {
              d_remaining_payload_bits = d_payload_bits;
          }
          in += d_total_subcarriers;
          ++consumed_items;
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (consumed_items);

        // Tell runtime system how many output items we produced.
        return produced_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

