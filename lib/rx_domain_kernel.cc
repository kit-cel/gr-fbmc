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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <fbmc/rx_domain_kernel.h>

namespace gr {
  namespace fbmc {

    rx_domain_kernel::rx_domain_kernel(std::vector<float> taps, int L, int overlap):
        d_taps(taps), d_L(L), d_overlap(overlap)
    {
    }

    rx_domain_kernel::~rx_domain_kernel()
    {
    }

    int
    rx_domain_kernel::generic_work(gr_complex* out, const gr_complex* in,
                                   int noutput_items)
    {
      return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

