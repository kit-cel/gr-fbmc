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
    multichannel_frame_generator_bvc::make(int used_subcarriers, int total_subcarriers, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
          (new multichannel_frame_generator_bvc_impl(used_subcarriers, total_subcarriers, payload_symbols, payload_bits, overlap, channel_map, preamble));
    }

    /*
     * The private constructor
     */
    multichannel_frame_generator_bvc_impl::multichannel_frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers,
                                                                           int payload_symbols, int payload_bits, int overlap,
                                                                           std::vector<int> channel_map,
                                                                           std::vector<gr_complex> preamble) :
        gr::block("multichannel_frame_generator_bvc", gr::io_signature::make(1, 1, sizeof(char)),
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * total_subcarriers)),
        d_used_subcarriers(used_subcarriers), d_total_subcarriers(total_subcarriers),
        d_payload_symbols(payload_symbols), d_payload_bits(payload_bits),
        d_overlap(overlap), d_subchannel_map(channel_map),
        d_preamble(preamble), d_num_subchannels(4), d_blocked_subchannel(0), d_CTS(false)
    {
      // 2 times overlap because we need 'zero-symbols' after preamble and after payload.
      d_preamble_symbols = d_preamble.size() / d_total_subcarriers; // number of complete symbol vectors
      d_frame_len = d_preamble_symbols + d_payload_symbols + 2 * d_overlap;

      // prepare all possible preamble and channel map setups
      setup_preamble();
      setup_channel_map();

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
      for(int i=0; i<d_preamble_buf.size(); i++)
      {
        volk_free(d_preamble_buf[i]);
      }
    }

    const gr_complex multichannel_frame_generator_bvc_impl::D_CONSTELLATION[2] = {gr_complex(-1.0f / std::sqrt(2.0f), 0.0f), gr_complex(1.0f / std::sqrt(2.0f), 0.0f)};

    void
    multichannel_frame_generator_bvc_impl::process_msg(pmt::pmt_t msg)
    {
      if(!pmt::is_pair(msg))
      {
        std::runtime_error("Received msg is not of type pair");
      }
      d_blocked_subchannel = pmt::to_long(pmt::cdr(msg));
      d_CTS = true;
    }
    void
    multichannel_frame_generator_bvc_impl::setup_preamble()
    {
      if(d_preamble.size() % d_total_subcarriers != 0) {
        throw std::runtime_error("Parameter mismatch: size(preamble) % total_subcarriers != 0");
      }

      for(int i=0; i<d_num_subchannels; i++) // prepare all 4 possibilities
      {
        d_preamble_buf.push_back( (gr_complex*) volk_malloc(sizeof(gr_complex) * d_preamble_symbols * d_total_subcarriers * d_num_subchannels, volk_get_alignment()) );

        for(int k=0; k<d_preamble_symbols; k++)
        {
          for(int n=0; n<d_num_subchannels; n++)
          {
            gr_complex *dest = d_preamble_buf[i]+k*d_total_subcarriers;
            gr_complex *src = &d_preamble[0]+k*d_total_subcarriers;
            int len = sizeof(gr_complex)*d_total_subcarriers;
            if(k==i) // current index is the blocked subchannel, copy zeros (== disable channel)
            {
              memset(dest, 0, len);
            }
            else // current index is a used subchannel, copy preamble symbols
            {
              memcpy(dest, src, len);
            }
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
      d_subchannel_map_ind.push_back(std::vector<int>());
      d_subchannel_map_ind.push_back(std::vector<int>());

      bool inphase = true;
      for(int i = 0; i < d_total_subcarriers; i++){
        if(d_subchannel_map[i] == 1){
          if(inphase){
            d_subchannel_map_ind[0].push_back(i);
          }
          else{
            d_subchannel_map_ind[1].push_back(i);
          }
        }
        if(i % 2 == 0){
          inphase = !inphase;
        }
      }
    }

    void
    multichannel_frame_generator_bvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      // always only 1 input and 1 output.
      ninput_items_required[0] = d_payload_bits * (d_num_subchannels - 1);
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_preamble(gr_complex* out)
    {
      memcpy(out, d_preamble_buf[d_blocked_subchannel] + d_total_subcarriers * d_num_subchannels * d_preamble_symbols, sizeof(gr_complex) * d_total_subcarriers * d_num_subchannels * d_preamble_symbols);
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_padding_zeros(gr_complex* out){
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers * d_num_subchannels * d_overlap);
    }

    inline void
    multichannel_frame_generator_bvc_impl::insert_payload(gr_complex* out, const char* inbuf)
    {
      // null output buffer
      memset(out, 0, sizeof(gr_complex) * d_total_subcarriers * d_payload_symbols * d_num_subchannels);

      for(int i=0; i<d_num_subchannels; i++)
      {
        if(i == d_blocked_subchannel)
        {
          i++; // skip the blocked subchannel
        }

        int frame_pos = d_preamble_symbols + d_overlap;
        for(int k=0; k<d_payload_symbols; k++)
        {
          int sel = inphase_selector(frame_pos);
          for (int n = 0; n < d_subchannel_map_ind[sel].size(); n++)
          {
            out[k * d_total_subcarriers * d_num_subchannels + // complete symbols (all subchannels)
                 i * d_total_subcarriers +                     // complete subchannels
                 d_subchannel_map_ind[sel][n]] = D_CONSTELLATION[*inbuf++];
          }
          frame_pos++;
        }
      }
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

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each(d_payload_bits*(d_num_subchannels-1));

      // Tell runtime system how many output items we produced.
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */