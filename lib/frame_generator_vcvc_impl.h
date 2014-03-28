/* -*- c++ -*- */
/* 
 * Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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

#ifndef INCLUDED_FBMC_FRAME_GENERATOR_VCVC_IMPL_H
#define INCLUDED_FBMC_FRAME_GENERATOR_VCVC_IMPL_H

#include <fbmc/frame_generator_vcvc.h>

namespace gr {
  namespace fbmc {

    class frame_generator_vcvc_impl : public frame_generator_vcvc
    {
     private:
      int d_sym_len; // length of one symbol aka the input vector length aka number of subcarriers
      int d_frame_len; // number of symbols (vectors) per frame
      int d_overlap; // hard-coded to 4
      int d_num_payload_sym; // number of payload symbols
      int d_payload_sym_ctr; // payload symbol counter to detect the start/end of frame

     public:
      frame_generator_vcvc_impl(int sym_len, int frame_len);
      ~frame_generator_vcvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_GENERATOR_VCVC_IMPL_H */

