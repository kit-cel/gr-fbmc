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
        d_total_subcarriers(total_subcarriers),
        d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_preamble_symbols(num_preamble_symbols), d_subchannel_map(channel_map)
    {
      setup_channel_map();
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
    multichannel_deframer_vcb_impl::setup_channel_map()
    {
      if(d_subchannel_map.size() != d_total_subcarriers) {
        throw std::runtime_error("Parameter mismatch: size(channel_map) != total_subcarriers");
      }

      // create the characteristic time-frequency pattern with alternating real and imaginary symbols

      // save indices of used subchannels
      for(int i=0; i<d_subchannel_map.size(); i++)
      {
        if(d_subchannel_map[i] != 0) {
          d_subchannel_map_index.push_back(i);
        }
      }

      std::vector<int> subchannel_map_offset_even;
      std::vector<int> subchannel_map_offset_odd;
      for(int i=0; i<d_subchannel_map_index.size(); i++)
      {
        subchannel_map_offset_even.push_back(d_subchannel_map_index[i] % 2);
        subchannel_map_offset_odd.push_back((d_subchannel_map_index[i]+1) % 2);
      }

      d_subchannel_map_offset.push_back(subchannel_map_offset_even);
      d_subchannel_map_offset.push_back(subchannel_map_offset_odd);
    }

    std::vector<bool>
    multichannel_deframer_vcb_impl::get_occupied_channels_from_tag(const gr_complex* inptr)
    {
      return std::vector<bool>(false, d_num_subchannels); // FIXME implement this
    }

    int
    multichannel_deframer_vcb_impl::extract_bits(char* outbuf, gr_complex* inbuf, std::vector<bool> blocked_subchannels)
    {
      int bits_written_total = 0;
      for(int i = 0; i < d_num_subchannels; i++)
      {
        std::cout << "deframer: extract bits: ";
        int bits_written = 0;
        int frame_pos = d_preamble_symbols + d_payload_symbols;
//        std::cout << "blocked_subchannels[i]: " << blocked_subchannels[i] << std::endl;
        if(!blocked_subchannels[i])
        {
          float* in_cast = (float*) inbuf;
          float *inptr = in_cast + 2 * i * d_total_subcarriers; // initial position pointing to the current subchannel
          //FIXME : line below causes empty space in the output buffer, introduce additional counter to avoid this
          char *outptr = outbuf + i * d_payload_bits; // same as above, output frames shall appear serially in the output buffer

          // process fully occupied symbols
          for (int k = 0; k < d_payload_symbols - 1; k++)
          {
            int sel = inphase_selector(frame_pos++);
            for(int n = 0; n < d_subchannel_map_index.size(); n++)
            {
              if(inptr[2*d_subchannel_map[n] + d_subchannel_map_offset[sel][n]] > 0.0f)
              {
                *outptr++ = 1;
              }
              else
              {
                *outptr++ = 0;
              }
              std::cout << int(*(outptr-1));
              inptr += 2 * d_total_subcarriers * d_num_subchannels; // move inptr to the subchannel's next symbol
            }
            bits_written += d_subchannel_map_index.size();
          }

          // process the last, possibly only partly occupied symbol
          int sel = inphase_selector(frame_pos);
          int remaining_bits = d_payload_bits - bits_written;
//          std::cout << "deframer: bits_written: " << bits_written << std::endl;
//          std::cout << "deframer: remaining bits: " << remaining_bits << std::endl;
          for(int n = 0; n < remaining_bits; n++)
          {
            if(inptr[2*d_subchannel_map[n] + d_subchannel_map_offset[sel][n]] > 0.0f)
            {
              *outptr++ = 1;
            }
            else
            {
              *outptr++ = 0;
            }
            std::cout << int(*(outptr-1));
          }
          bits_written += remaining_bits;
          std::cout << std::endl;
//          std::cout << "deframer: user " << i << " bits_written: " << bits_written << std::endl;
          bits_written_total += bits_written;
        }
//        else{ std::cout << "extract_bits(): skip blocked channel " << i << std::endl; }
      }
      return bits_written_total;
    }

    int
    multichannel_deframer_vcb_impl::general_work (int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      gr_complex *in = (gr_complex *) input_items[0];
      char *out = (char *) output_items[0];

      std::cout << "deframer: process one frame" << std::endl;

      // skip the preamble and the following overlap FIXME: make sure this works even after the filterbank
      out += d_total_subcarriers * d_num_subchannels * (d_overlap + d_preamble_symbols);
      std::vector<bool> blocked_subcarriers = get_occupied_channels_from_tag(in);
      int nbits = extract_bits(out, in, blocked_subcarriers);

      consume_each(d_frame_len * d_total_subcarriers * d_num_subchannels);
      return nbits;

    }

  } /* namespace fbmc */
} /* namespace gr */

