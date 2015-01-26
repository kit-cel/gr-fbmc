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
#include "rx_domain_cvc_impl.h"

namespace gr {
  namespace fbmc {

    rx_domain_cvc::sptr
    rx_domain_cvc::make(std::vector<float> taps, int L)
    {
      return gnuradio::get_initial_sptr(new rx_domain_cvc_impl(taps, L));
    }

    /*
     * The private constructor
     */
    rx_domain_cvc_impl::rx_domain_cvc_impl(std::vector<float> taps, int L) :
        gr::sync_decimator("rx_domain_cvc",
                           gr::io_signature::make(1, 1, sizeof(gr_complex)),
                           gr::io_signature::make(1, 1, sizeof(gr_complex) * L),
                           L / 2), rx_domain_kernel(taps, L)
    {
      set_output_multiple(overlap() * 2);
    }

    /*
     * Our virtual destructor.
     */
    rx_domain_cvc_impl::~rx_domain_cvc_impl()
    {
    }

    int
    rx_domain_cvc_impl::work(int noutput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      /* make sure enough input samples are available for a big FFT.
       * e.g. for L = 32 and overlap = 4
       * FFT size: overlap * L = 128
       * in-out ratio: 16:1
       * --> noutput_items must be >= 8
       *    --> if noutput_items = 8 --> 128 input_items available
       * now +16 input_items for each additional output_item needed
       * thus subtract 2 * overlap - 1 from noutput_items.
       */

      noutput_items = std::max(0, noutput_items - 2 * overlap() + 1);
      std::cout << "generic_work noutput_items = " << noutput_items << std::endl;

      int nout = rx_domain_kernel::generic_work(out, in, noutput_items);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace fbmc */
} /* namespace gr */

