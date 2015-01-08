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

#ifndef INCLUDED_FBMC_APPLY_BETAS_VCVC_IMPL_H
#define INCLUDED_FBMC_APPLY_BETAS_VCVC_IMPL_H

#include <fbmc/apply_betas_vcvc.h>

namespace gr {
  namespace fbmc {

    class apply_betas_vcvc_impl : public apply_betas_vcvc
    {
     private:
      int d_L; // number of subcarriers (some are possibly zero)
      std::vector< std::vector< gr_complex* > > d_betas_al;
      int d_sym_ctr; // counter to select the correct beta vector for each input vector.
      int d_inverse; // tells the block which table is to be used

      inline void apply_betas_to_one_vector(gr_complex* out, const gr_complex* in);

     public:
      apply_betas_vcvc_impl(int L, int inverse);
      ~apply_betas_vcvc_impl();

      // Where all the action really happens
      int
      work(int noutput_items, gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };
  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_APPLY_BETAS_VCVC_IMPL_H */

