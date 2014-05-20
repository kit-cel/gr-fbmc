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

namespace gr {
  namespace fbmc {

    class preamble_insertion_vcvc_impl : public preamble_insertion_vcvc
    {
     private:
      int d_L; // num subcarriers
      int d_frame_len; // num symbols per frame
      std::string d_type; // characterizes equalization method
      int d_ctr; // counts the symbols
      int d_overlap; // onum overlapping symbols  
      int d_num_equal_sym; // number of equal transmit (!) symbols visible at the receiver  
      int d_num_preamble_sym; // total preamble length (without tailing 0 symbols)  

     public:
      preamble_insertion_vcvc_impl(int L, int frame_len, std::string type, int overlap);
      ~preamble_insertion_vcvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_IMPL_H */

