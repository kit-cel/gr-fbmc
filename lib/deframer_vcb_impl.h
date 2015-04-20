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

#ifndef INCLUDED_FBMC_DEFRAMER_VCB_IMPL_H
#define INCLUDED_FBMC_DEFRAMER_VCB_IMPL_H

#include <fbmc/deframer_vcb.h>

namespace gr {
  namespace fbmc {

    class deframer_vcb_impl : public deframer_vcb
    {
     private:
      int d_used_subcarriers;
      int d_total_subcarriers;
      int d_payload_symbols;
      int d_overlap;
      std::vector<std::vector<int> > d_channel_map;
      int d_preamble_symbols;
      int d_frame_len;
      int d_frame_position;
      int d_start_of_payload;
      int d_end_of_payload;

      void setup_channel_map(std::vector<int> channel_map);

      int extract_bytes(char* out, const gr_complex* inbuf);

     public:
      deframer_vcb_impl(int used_subcarriers, int total_subcarriers, int num_preamble_symbols, int payload_symbols, int overlap, std::vector<int> channel_map);
      ~deframer_vcb_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_DEFRAMER_VCB_IMPL_H */

