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

#ifndef INCLUDED_FBMC_FRAME_GENERATOR_BVC_IMPL_H
#define INCLUDED_FBMC_FRAME_GENERATOR_BVC_IMPL_H

#include <fbmc/frame_generator_bvc.h>

namespace gr {
  namespace fbmc {

    class frame_generator_bvc_impl : public frame_generator_bvc
    {
     private:
      int d_used_subcarriers;
      int d_total_subcarriers;
      int d_payload_symbols;
      int d_overlap;
      std::vector<int> d_channel_map;
      std::vector<gr_complex> d_preamble;
      int d_preamble_symbols;
      int d_frame_len;
      int d_frame_position;

      void insert_preamble_vector(gr_complex* out, int preamble_position);

     public:
      frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers, int payload_symbols, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble);
      ~frame_generator_bvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_GENERATOR_BVC_IMPL_H */

