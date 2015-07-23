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

#ifndef INCLUDED_MULTICHANNEL_FBMCFRAME_GENERATOR_BVC_IMPL_H
#define INCLUDED_MULTICHANNEL_FBMCFRAME_GENERATOR_BVC_IMPL_H

#include <fbmc/multichannel_frame_generator_bvc.h>

namespace gr {
  namespace fbmc {

    class multichannel_frame_generator_bvc_impl : public multichannel_frame_generator_bvc
    {
    private:
      int d_used_subcarriers; // per subchannel
      int d_total_subcarriers; // per subchannel
      int d_payload_symbols; // per subchannel
      int d_payload_bits; // per subchannel
      int d_overlap;
      std::vector<int> d_subchannel_map;
      std::vector<std::vector<int> > d_subchannel_map_ind;
      std::vector<gr_complex> d_preamble;
      std::vector<gr_complex*> d_preamble_buf;
      int d_preamble_symbols;
      int d_frame_len;
      int d_num_subchannels;
      int d_blocked_subchannel;
      bool d_CTS;

      static const gr_complex D_CONSTELLATION[2];

      void process_msg(pmt::pmt_t msg);
      void setup_preamble();
      void setup_channel_map(); // blocked channel index ranges from 0 to d_num_subchannels-1
      std::vector<gr_complex> setup_constellation() const;

      inline void insert_preamble(gr_complex* out);

      inline void insert_padding_zeros(gr_complex* out);

      inline void insert_payload(gr_complex* out, const char* inbuf);

      inline int inphase_selector(int pos) const {return (pos - d_preamble_symbols + d_overlap) % 2;};

    public:
      multichannel_frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble);
      ~multichannel_frame_generator_bvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_MULTICHANNEL_FBMCFRAME_GENERATOR_BVC_IMPL_H */

