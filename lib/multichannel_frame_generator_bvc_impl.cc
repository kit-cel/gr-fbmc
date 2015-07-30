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
#include "multichannel_frame_generator_bvc_impl.h"

namespace gr {
  namespace fbmc {

    multichannel_frame_generator_bvc::sptr
    multichannel_frame_generator_bvc::make(int total_subcarriers, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
          (new multichannel_frame_generator_bvc_impl(total_subcarriers, payload_symbols, payload_bits, overlap, channel_map, preamble));
    }

    /*
     * The private constructor
     */
    multichannel_frame_generator_bvc_impl::multichannel_frame_generator_bvc_impl(int total_subcarriers,
                                                                           int payload_symbols, int payload_bits, int overlap,
                                                                           std::vector<int> channel_map,
                                                                           std::vector<gr_complex> preamble) :
        gr::block("multichannel_frame_generator_bvc", gr::io_signature::make(1, 1, sizeof(char)),
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers * d_num_subchannels)),
        d_total_subcarriers(total_subcarriers), d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_subchannel_map(channel_map), d_num_used_subchannels(0),
        d_preamble(preamble), d_CTS(false)
    {
      // 2 times overlap because we need 'zero-symbols' after preamble and after payload.
      d_preamble_symbols = d_preamble.size() / d_total_subcarriers; // number of complete symbol vectors
      d_frame_len = d_preamble_symbols + d_payload_symbols + 2 * d_overlap;

      // prepare preamble buffer and subchannel map
      d_preamble_buf = new gr_complex[d_num_subchannels*d_total_subcarriers*d_preamble_symbols];
      setup_channel_map();
      d_blocked_subchannels = std::vector<bool>();

      pmt::pmt_t CTS_PORT = pmt::mp("CTS_in");
      message_port_register_in(CTS_PORT);
      set_msg_handler(CTS_PORT, boost::bind(&multichannel_frame_generator_bvc_impl::process_msg, this, _1));
      set_output_multiple(d_frame_len);
    }

    /*
     * Our virtual destructor.
     */
    multichannel_frame_generator_bvc_impl::~multichannel_frame_generator_bvc_impl()
    {
      delete[] d_preamble_buf;
    }

    const float multichannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {-1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f)};

    void
    multichannel_frame_generator_bvc_impl::process_msg(pmt::pmt_t msg)
    {
      if(!pmt::is_dict(msg))
      {
        throw std::runtime_error("Wrong message type, expected dict");
      }

      if(d_CTS) // previous frame has not been sent yet, drop it in favor of the newer one as we are obviously late
      {
        std::cout << "WARNING: CTS messages are stacking up, dropping previous CTS" << std::endl;
      }

      d_num_used_subchannels = d_num_subchannels;
      d_blocked_subchannels.clear();
      for (int i = 0; i < d_num_subchannels; i++) {
        if (pmt::dict_ref(msg, pmt::mp(i), pmt::PMT_F) == pmt::PMT_T) {
          d_blocked_subchannels.push_back(true);
          d_num_used_subchannels--;
        }
        else
        {
          d_blocked_subchannels.push_back(false);
        }
      }

//      std::cout << std::endl << "CCA report: " << std::endl;
//      for(int i = 0; i < d_num_subchannels; i++)
//      {
//        std::cout << " -- Channel " << i << " blocked? -> " << d_blocked_subchannels[i] << std::endl;
//      }

      if(d_num_used_subchannels > 0) // at least one subchannel must be free
      {
        d_CTS = true;
        setup_preamble();
      }
      else
      {
//        std::cout << std::endl << "All channels blocked, disable CTS" << std::endl;
        d_CTS = false;
      }
    }

    void
    multichannel_frame_generator_bvc_impl::setup_preamble()
    {
      if(d_preamble.size() % d_total_subcarriers != 0) {
        throw std::runtime_error("Parameter mismatch: size(preamble) % total_subcarriers != 0");
      }

      memset(d_preamble_buf, 0, sizeof(gr_complex) * d_preamble_symbols * d_total_subcarriers * d_num_subchannels);
      for(int i=0; i<d_preamble_symbols; i++)
      {
        for(int k=0; k<d_num_subchannels; k++)
        {
          gr_complex *dest = &d_preamble_buf[i * d_total_subcarriers * d_num_subchannels + k * d_total_subcarriers];
          gr_complex *src = &d_preamble[i * d_total_subcarriers];
          int len = sizeof(gr_complex) * d_total_subcarriers;
          if (d_blocked_subchannels[k]) // current index is a blocked subchannel, copy zeros (== disable channel)
          {
//            std::cout << "preamble_setup(): skip subchannel " << k << std::endl;
            memset(dest, 0, len);
          }
          else // current index is a used subchannel, copy preamble symbols
          {
            memcpy(dest, src, len);
          }
        }
      }
    }

    void
    multichannel_frame_generator_bvc_impl::setup_channel_map()
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

    void
    multichannel_frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      // always only 1 input and 1 output.
      ninput_items_required[0] = d_payload_bits * d_num_subchannels; // This could probably be reduced to d_num_subchannels - num_blocked_channels
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_preamble(gr_complex*& out)
    {
      memcpy(out, d_preamble_buf, sizeof(gr_complex) * d_total_subcarriers * d_num_subchannels * d_preamble_symbols);
      out += d_total_subcarriers * d_num_subchannels * d_preamble_symbols;
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_padding_zeros(gr_complex*& out){
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers * d_num_subchannels * d_overlap);
      out += d_total_subcarriers * d_num_subchannels * d_overlap;
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_payload(gr_complex*& out, const char* inbuf)
    {
      // null output buffer
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers * d_payload_symbols * d_num_subchannels);

      // cast output buffer pointer to float to be able to access the real and imaginary part via pointer arithmetics
      float* outptr = (float*) out;
      for(int i=0; i<d_num_subchannels; i++)
      {
        int bits_written = 0;
        if(!d_blocked_subchannels[i]) // skip the blocked subchannel
        {
          int frame_pos = d_preamble_symbols + d_overlap;
          for(int k=0; k<d_payload_symbols-1; k++) // handle all fully occupied symbols
          {
            int sel = inphase_selector(frame_pos);
            for(int n = 0; n < d_subchannel_map_index.size(); n++)
            {
//              std::cout << "framer: write offset " << 2*(k * d_total_subcarriers * d_num_subchannels + i * d_total_subcarriers + d_subchannel_map_index[n]) + d_subchannel_map_offset[sel][n] << std::endl;
              outptr[ 2*(k * d_total_subcarriers * d_num_subchannels +  // complete symbols (all subchannels)
                   i * d_total_subcarriers +                         // complete subchannels
                   d_subchannel_map_index[n]) +                      // subcarrier index
                   d_subchannel_map_offset[sel][n] ]                 // I/Q offset
                   = D_CONSTELLATION[*inbuf++];
              bits_written++;
            }
            frame_pos++;
          }
          // handle the last, possibly only partly occupied symbol
          int remaining_bits = d_payload_bits - bits_written;
          int sel = inphase_selector(frame_pos);
          for(int n=0; n<remaining_bits; n++)
          {
            outptr[2*((d_payload_symbols-1) * d_total_subcarriers * d_num_subchannels +  // N-1 complete symbols (all subchannels)
                i * d_total_subcarriers +                                             // complete subchannels
                d_subchannel_map_index[n]) +                                          // subcarrier index
                d_subchannel_map_offset[sel][n] ]                                     // I/Q offset
                = D_CONSTELLATION[*inbuf++];
            bits_written++;
          }
//          std::cout << "insert_payload(): user " << i << " bits_written: " << bits_written << std::endl;
        }
//        else
//        {
//          std::cout << "framer: insert_payload(): skip subchannel " << i << std::endl;
//        }
      }
      out += d_total_subcarriers * d_payload_symbols * d_num_subchannels;
    }

    int
    multichannel_frame_generator_bvc_impl::general_work(int noutput_items, gr_vector_int &ninput_items,
                                                     gr_vector_const_void_star &input_items,
                                                     gr_vector_void_star &output_items)
    {
      // if there is no valid CTS, return
      if(!d_CTS)
      {
        consume_each(0);
        return 0;
      }

      const char *in = (const char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      insert_preamble(out);
      insert_padding_zeros(out);
      insert_payload(out, in);
      insert_padding_zeros(out);

      d_CTS = false;

      consume_each(d_payload_bits*d_num_used_subchannels);
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */