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
#include "multichannel_deframer_vcb_impl.h"

namespace gr {
  namespace fbmc {

    multichannel_deframer_vcb::sptr
    multichannel_deframer_vcb::make(int total_subcarriers, int num_preamble_symbols, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map)
    {
      return gnuradio::get_initial_sptr
          (new multichannel_deframer_vcb_impl(total_subcarriers, num_preamble_symbols, payload_symbols, payload_bits, overlap, channel_map));
    }

    /*
     * The private constructor
     */
    multichannel_deframer_vcb_impl::multichannel_deframer_vcb_impl(int total_subcarriers, int num_preamble_symbols,
                                         int payload_symbols, int payload_bits, int overlap,
                                         std::vector<int> channel_map) :
        gr::block("multichannel_deframer_vcb",
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers * 4),
                  gr::io_signature::make(1, 1, sizeof(char))),
        d_total_subcarriers(total_subcarriers), d_num_subchannels(4),
        d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_preamble_symbols(num_preamble_symbols)
    {
      setup_channel_map(channel_map);
      d_frame_len = d_preamble_symbols + d_overlap + d_payload_symbols + d_overlap;

      // can only set this in c'tor, so assume the maximum output size
      set_output_multiple(d_payload_bits * d_num_subchannels);
    }

    /*
     * Our virtual destructor.
     */
    multichannel_deframer_vcb_impl::~multichannel_deframer_vcb_impl()
    {
    }

    void
    multichannel_deframer_vcb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = d_frame_len * d_total_subcarriers * d_num_subchannels; // one complete frame
    }

    void
    multichannel_deframer_vcb_impl::setup_channel_map(std::vector<int> channel_map)
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

    std::vector<bool>
    multichannel_deframer_vcb_impl::get_occupied_channels_from_tag(const gr_complex* inptr)
    {
      return std::vector<bool>(false, d_num_subchannels); // FIXME implement this
    }

    int
    multichannel_deframer_vcb_impl::extract_bits(char* outbuf, gr_complex* inbuf, std::vector<bool> blocked_subchannels)
    {
      int bits_written = 0;
      for(int i = 0; i < d_num_subchannels; i++)
      {
        int frame_pos = d_preamble_symbols + d_payload_symbols;
        std::cout << "blocked_subchannels[i]: " << blocked_subchannels[i] << std::endl;
        if(!blocked_subchannels[i]) {
          gr_complex *inptr = inbuf + i * d_total_subcarriers; // initial offset pointing to the right subchannel
          char *outptr = outbuf + i * d_payload_bits; // same as above, output frames shall appear serially in the output buffer

          // process fully occupied symbols
          for (int k = 0; k < d_payload_symbols - 1; k++)
          {
            int sel = inphase_selector(frame_pos++);
            for(int n = 0; n < d_channel_map[sel].size(); n++)
            {
              if(inptr[d_channel_map[sel][n]].real() > 0.0f)
              {
                *outbuf++ = 1;
              }
              else
              {
                *outbuf++ = 0;
              }
              inptr += d_total_subcarriers * d_num_subchannels;
            }
            bits_written += d_channel_map[sel].size();
            std::cout << "bits_written: " << bits_written << std::endl;
          }

          // process the last, possibly only partly occupied symbol
          int sel = inphase_selector(frame_pos);
          int remaining_bits = d_payload_bits - bits_written;
          std::cout << "remaining bits: " << remaining_bits << std::endl;
          for(int n = 0; n < remaining_bits; n++)
          {
            if(inptr[d_channel_map[sel][n]].real() > 0.0f)
            {
              *outbuf++ = 1;
            }
            else
            {
              *outbuf++ = 0;
            }
          }
          bits_written += remaining_bits;
        }
        else{ std::cout << "extract_bits(): skip blocked channel " << i << std::endl; }
      }
      return bits_written;
    }

    int
    multichannel_deframer_vcb_impl::general_work (int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      gr_complex *in = (gr_complex *) input_items[0];
      char *out = (char *) output_items[0];

      // skip the preamble and the following overlap
      out += d_total_subcarriers * d_num_subchannels * (d_overlap + d_preamble_symbols);
      int frame_pos = d_preamble_symbols + d_overlap;
      std::vector<bool> blocked_subcarriers = get_occupied_channels_from_tag(in);
      int nbits = extract_bits(out, in, blocked_subcarriers);

      consume_each(d_frame_len * d_total_subcarriers * d_num_subchannels);
      return nbits;

    }

  } /* namespace fbmc */
} /* namespace gr */

