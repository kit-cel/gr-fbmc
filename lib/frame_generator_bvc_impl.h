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
      std::vector<std::vector<int> > d_channel_map;
      std::vector<gr_complex> d_preamble;
      gr_complex* d_preamble_buf;
      int d_preamble_symbols;
      int d_frame_len;
      int d_frame_position;

      const float D_INVSQRT;

      void setup_preamble(std::vector<gr_complex> preamble);
      void setup_channel_map(std::vector<int> channel_map);

      inline void insert_preamble_vector(gr_complex* out, int preamble_position);

      inline void insert_padding_zeros(gr_complex* out);

      inline int insert_payload(gr_complex* out, const char* inbuf);

      inline int inphase_selector() const {return (d_frame_position - d_preamble_symbols + d_overlap) % 2;};
      inline int nused_items_on_vector() const {return d_channel_map[inphase_selector()].size();};

     public:
      frame_generator_bvc_impl(int used_subcarriers, int total_subcarriers, int payload_symbols, int overlap, std::vector<int> channel_map, std::vector<gr_complex> preamble);
      ~frame_generator_bvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      std::vector<std::vector<int> > channel_map(){return d_channel_map;};

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_GENERATOR_BVC_IMPL_H */

