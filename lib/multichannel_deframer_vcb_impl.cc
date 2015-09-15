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
#include <volk/volk.h>

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
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers * d_num_subchannels),
                  gr::io_signature::make(1, 1, sizeof(char))),
        d_total_subcarriers(total_subcarriers),
        d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_preamble_symbols(num_preamble_symbols), d_subchannel_map(channel_map)
    {
      setup_channel_map();
      d_frame_len = d_preamble_symbols + d_overlap + d_payload_symbols + 2*d_overlap;

      // can only set this in c'tor, so assume the maximum output size
      set_output_multiple(d_payload_bits * d_num_subchannels);

      std::cout << "NOTE: deframer writes a debug file!" << std::endl;
      d_file = fopen("deframer_soft.bin", "wb");
      if(d_file == NULL)
        throw std::runtime_error("File not open");
    }

    /*
     * Our virtual destructor.
     */
    multichannel_deframer_vcb_impl::~multichannel_deframer_vcb_impl()
    {
      fclose(d_file);
    }

    void
    multichannel_deframer_vcb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = d_frame_len; // one complete frame
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
      // FIXME: use consistent notation for occupied/used/busy/blocked subchannels
      std::vector<gr::tag_t> tags;
      get_tags_in_window(tags, 0, 0, 1, pmt::mp("occupied_subchannels")); // tag must be located on the first sample
      if(tags.empty())
      {
        throw std::runtime_error("No tag found");
      }
      if(tags.size() > 1)
      {
        throw std::runtime_error("Found multiple tags");
      }
      pmt::pmt_t vec = tags[0].value;
      if(!pmt::is_vector(vec))
      {
        throw std::runtime_error("Expected a PMT vector");
      }

      std::vector<bool> ret(d_num_subchannels, false);
      for(int i=0; i<d_num_subchannels; i++) {
        if (pmt::vector_ref(vec, i) == pmt::PMT_T) {
          ret[i] = false;
        }
        else if (pmt::vector_ref(vec, i) == pmt::PMT_F) {
          ret[i] = true;
        }
        else {
          throw std::runtime_error("Invalid value");
        }
//        std::cout << "deframer: channel " << i << " used: " << ret[i] << std::endl;
      }
      return ret;
    }

    void
    multichannel_deframer_vcb_impl::get_corrcoefs_from_tag(const gr_complex* inptr)
    {
      std::vector<gr::tag_t> tags;
      get_tags_in_window(tags, 0, 0, 1, pmt::mp("corr_coefs")); // tag must be located on the first sample
      if(tags.empty())
      {
        throw std::runtime_error("No tag found");
      }
      if(tags.size() > 1)
      {
        throw std::runtime_error("Found multiple tags");
      }
      pmt::pmt_t vec = tags[0].value;
      if(!pmt::is_vector(vec))
      {
        throw std::runtime_error("Expected a PMT vector");
      }

      d_corrcoefs.clear();
      int len = pmt::length(vec);
      for(int i=0; i<len; i++)
      {
        d_corrcoefs.push_back(gr_complex(pmt::to_complex(pmt::vector_ref(vec, i))));
      }
    }

    void
    multichannel_deframer_vcb_impl::correct_phase_offset(gr_complex* buf, std::vector<bool> blocked_subchannels)
    {
      int subchannel_ctr = 0;
      for(int i=0; i<d_num_subchannels; i++)
      {
        gr_complex* startptr = buf + (d_preamble_symbols + d_overlap)*d_total_subcarriers*d_num_subchannels;
        if(!blocked_subchannels[i])
        {
          gr_complex phi = d_corrcoefs[subchannel_ctr]/std::abs(d_corrcoefs[subchannel_ctr]);
          for(int k=0; k<d_payload_symbols; k++)
          {
            gr_complex* pos = startptr+k*d_total_subcarriers*d_num_subchannels + i*d_total_subcarriers;
            volk_32fc_s32fc_multiply_32fc(pos, pos, phi, d_total_subcarriers);
          }
          subchannel_ctr++;
        }
      }
    }

    int
    multichannel_deframer_vcb_impl::extract_bits(char* outbuf, gr_complex* inbuf, std::vector<bool> blocked_subchannels)
    {
      int bits_written_total = 0;
      int outframe_ctr = 0;
      for(int i = 0; i < d_num_subchannels; i++)
      {
        int bits_written = 0;
        int frame_pos = d_preamble_symbols + d_overlap;
        if(!blocked_subchannels[i])
        {
          float* inptr = (float*) inbuf;
          char *outptr = outbuf + outframe_ctr * d_payload_bits; // same as above, output frames shall appear serially in the output buffer
          outframe_ctr++; // only increase counter if channel is not blocked, avoiding gaps in the output buffer

          // process fully occupied symbols
          for (int k = 0; k < d_payload_symbols - 1; k++)
          {
            int sel = inphase_selector(frame_pos++);
            for(int n = 0; n < d_subchannel_map_index.size(); n++)
            {
              float pam_symbol = inptr[2*(k * d_total_subcarriers * d_num_subchannels +  // complete symbols (all subchannels)
                                       i * d_total_subcarriers +                         // complete subchannels
                                       d_subchannel_map_index[n]) +                      // subcarrier index
                                       d_subchannel_map_offset[sel][n]];
              fwrite(&pam_symbol, sizeof(float), 1, d_file);
              if(pam_symbol > 0.0f)
              {
                *outptr++ = 1;
              }
              else
              {
                *outptr++ = 0;
              }
            }
            bits_written += d_subchannel_map_index.size();
          }

          // process the last, possibly only partly occupied symbol
          int sel = inphase_selector(frame_pos);
          int remaining_bits = d_payload_bits - bits_written;
          for(int n = 0; n < remaining_bits; n++)
          {
            float pam_symbol = inptr[2*((d_payload_symbols-1) * d_total_subcarriers * d_num_subchannels +  // complete symbols (all subchannels)
                                        i * d_total_subcarriers +                         // complete subchannels
                                        d_subchannel_map_index[n]) +                      // subcarrier index
                                     d_subchannel_map_offset[sel][n]];
            if(pam_symbol > 0.0f)
            {
              *outptr++ = 1;
            }
            else
            {
              *outptr++ = 0;
            }
          }
          bits_written += remaining_bits;
          bits_written_total += bits_written;
        }
//        else{ std::cout << "deframer: extract_bits(): skip blocked channel " << i << std::endl; }
      }
      return /*bits_written_total*/ outframe_ctr * d_payload_bits;
    }

    int
    multichannel_deframer_vcb_impl::general_work (int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      gr_complex *in = (gr_complex *) input_items[0];
      char *out = (char *) output_items[0];

      // skip the preamble and the following overlap FIXME: make sure this works even after the filterbank
      in += d_total_subcarriers * d_num_subchannels * (d_overlap + d_preamble_symbols);
      std::vector<bool> blocked_subcarriers = get_occupied_channels_from_tag(in);
      get_corrcoefs_from_tag(in);
      correct_phase_offset(in, blocked_subcarriers);
      int nbits = extract_bits(out, in, blocked_subcarriers);

      consume_each(d_frame_len);
      return nbits;

    }

  } /* namespace fbmc */
} /* namespace gr */

