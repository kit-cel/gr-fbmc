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

#ifndef INCLUDED_FBMC_MULTICHANNEL_DEFRAMER_VCB_IMPL_H
#define INCLUDED_FBMC_MULTICHANNEL_DEFRAMER_VCB_IMPL_H

#include <fbmc/multichannel_deframer_vcb.h>
#include <cstdio>

namespace gr {
  namespace fbmc {

    class multichannel_deframer_vcb_impl : public multichannel_deframer_vcb
    {
    private:
      int d_total_subcarriers;
      int d_payload_symbols;
      int d_payload_bits;
      int d_overlap;
      std::vector<int> d_subchannel_map;
      std::vector<int> d_subchannel_map_index;
      std::vector<std::vector<int> > d_subchannel_map_offset;
      static const int d_num_subchannels = 4;
      int d_preamble_symbols;
      int d_frame_len;
      std::vector<gr_complex> d_corrcoefs;

      void setup_channel_map();

      void correct_phase_offset(gr_complex* buf, std::vector<bool> blocked_subchannels);

      int extract_bits(char* out, gr_complex* inbuf, std::vector<bool> blocked_subchannels);
      std::vector<bool> get_occupied_channels_from_tag(const gr_complex* inptr);
      void get_corrcoefs_from_tag(const gr_complex* inptr);

      FILE* d_file1;
      FILE* d_file2;

      inline int inphase_selector(int pos) const {return (pos - d_preamble_symbols + d_overlap) % 2;};

    public:
      multichannel_deframer_vcb_impl(int total_subcarriers, int num_preamble_symbols, int payload_symbols, int payload_bits, int overlap, std::vector<int> channel_map);
      ~multichannel_deframer_vcb_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_MULTICHANNEL_DEFRAMER_VCB_IMPL_H */

