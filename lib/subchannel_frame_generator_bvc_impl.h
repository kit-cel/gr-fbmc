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
      void insert_preamble();
      void insert_pilots();
      void insert_payload(const char* inbuf, unsigned int* bits_written);
      void insert_aux_pilots(const unsigned int N, const unsigned int K);

      void init_freq_time_frame();
      void write_output(gr_complex*& out);

      int d_subcarriers, d_payload_symbols, d_payload_bits, d_overlap, d_frame_len, d_pilot_timestep, d_guard_carriers;
      int d_num_zeros;
      float d_pilot_amp;
      std::vector<gr_complex> d_preamble_symbols;
      std::vector<int> d_pilot_carriers, d_data_carriers;
      static const float d_weights_ee[3][7];
      static const float d_weights_eo[3][7];
      static const float d_weights_oe[3][7];
      static const float d_weights_oo[3][7];
      std::vector<std::vector<float> > d_freq_time_frame;
      bool d_padding;



      static const float D_CONSTELLATION[2];

     public:
      subchannel_frame_generator_bvc_impl(int subcarriers, int guard_carriers,
                                          int payload_bits, int overlap,
                                          std::vector<gr_complex> preamble_symbols,
                                          float pilot_amp, int pilot_timestep,
                                          std::vector<int> pilot_carriers, int frame_len, bool padding);
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

