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

#ifndef INCLUDED_FBMC_INPUT_COMMUTATOR_CVC_IMPL_H
#define INCLUDED_FBMC_INPUT_COMMUTATOR_CVC_IMPL_H

#include <fbmc/input_commutator_cvc.h>
#include <valarray>

namespace gr {
  namespace fbmc {

    class input_commutator_cvc_impl : public input_commutator_cvc
    {
     private:
      int d_L; // output vector length
      int d_ctr; // counter pointing to the next position in the output vector
      std::valarray<gr_complex> d_buf; // buffer for samples between calls to work()

     public:
      input_commutator_cvc_impl(int L);
      ~input_commutator_cvc_impl();
      
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      // Where all the action really happens
      int general_work(int noutput_items,
			gr_vector_int &ninput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_INPUT_COMMUTATOR_CVC_IMPL_H */

