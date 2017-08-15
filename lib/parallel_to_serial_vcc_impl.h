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

#ifndef INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_IMPL_H
#define INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_IMPL_H

#include <fbmc/parallel_to_serial_vcc.h>

namespace gr {
  namespace fbmc {

    class parallel_to_serial_vcc_impl : public parallel_to_serial_vcc
    {
     private:
      int d_len_out; // number of samples extracted from each vector
      int d_vlen_in; // length of input vector
      std::vector<int> d_channel_map; // channel occupancy
      void inline unmap_one_symbol(gr_complex* out, const gr_complex* in);

     public:
      parallel_to_serial_vcc_impl(int len_out, int vlen_in, std::vector<int> channel_map);
      ~parallel_to_serial_vcc_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_IMPL_H */

