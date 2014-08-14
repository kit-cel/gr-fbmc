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

#ifndef INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_IMPL_H
#define INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_IMPL_H

#include <fbmc/preamble_insertion_vcvc.h>
#include <cstdio>

namespace gr {
  namespace fbmc {

    class preamble_insertion_vcvc_impl : public preamble_insertion_vcvc
    {
     private:
      int d_L; // num subcarriers
      int d_frame_len; // num symbols per frame
      int d_ctr; // counts the symbols
      std::vector<gr_complex> d_preamble_sym; // preamble symbol
      FILE* dbg_fp;

     public:
      preamble_insertion_vcvc_impl(int frame_len, std::vector<gr_complex> preamble_sym);
      ~preamble_insertion_vcvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_IMPL_H */

