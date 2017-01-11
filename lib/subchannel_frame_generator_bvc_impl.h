/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_FBMC_SUBCHANNEL_FRAME_GENERATOR_BVC_IMPL_H
#define INCLUDED_FBMC_SUBCHANNEL_FRAME_GENERATOR_BVC_IMPL_H

#include <fbmc/subchannel_frame_generator_bvc.h>

namespace gr {
  namespace fbmc {

    class subchannel_frame_generator_bvc_impl : public subchannel_frame_generator_bvc
    {
     private:
      // Nothing to declare in this block.
      void insert_preamble(gr_complex*& out);
      void insert_pilots(gr_complex*& out);
      void insert_payload(gr_complex*& out, const char* inbuf);
      int d_subcarriers, d_payload_symbols, d_payload_bits, d_overlap, d_subchannels, d_frame_len, d_pilot_timestep;
      float d_pilot_amp;
      std::vector<float> d_preamble_symbols;
      std::vector<int> d_pilot_carriers;

      static const float D_CONSTELLATION[2];

     public:
      subchannel_frame_generator_bvc_impl(int subcarriers, int payload_symbols,
                                          int payload_bits, int overlap,
                                          int subchannels,
                                          std::vector<float> preamble_symbols,
                                          float pilot_amp, int pilot_timestep,
                                          std::vector<int> pilot_carriers);
      ~subchannel_frame_generator_bvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SUBCHANNEL_FRAME_GENERATOR_BVC_IMPL_H */

