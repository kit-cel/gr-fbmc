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

#ifndef INCLUDED_FBMC_RX_DOMAIN_CVC_IMPL_H
#define INCLUDED_FBMC_RX_DOMAIN_CVC_IMPL_H

#include <fbmc/rx_domain_cvc.h>
#include <fbmc/rx_domain_kernel.h>

namespace gr {
  namespace fbmc {

    class rx_domain_cvc_impl : public rx_domain_cvc, rx_domain_kernel
    {
    private:
      // Nothing to declare in this block.

    public:
      rx_domain_cvc_impl(std::vector<float> taps, int L, int overlap);
      ~rx_domain_cvc_impl();

      std::vector<float> taps(){return rx_domain_kernel::taps();};
      int L(){return rx_domain_kernel::L();};
      int overlap(){return rx_domain_kernel::overlap();};

      // Where all the action really happens
      int
      work(int noutput_items, gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_RX_DOMAIN_CVC_IMPL_H */

