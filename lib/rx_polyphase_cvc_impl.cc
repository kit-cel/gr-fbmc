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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "rx_polyphase_cvc_impl.h"

namespace gr {
  namespace fbmc {

    rx_polyphase_cvc::sptr
    rx_polyphase_cvc::make(std::vector<float> taps, int L)
    {
      return gnuradio::get_initial_sptr(new rx_polyphase_cvc_impl(taps, L));
    }

    /*
     * The private constructor
     */
    rx_polyphase_cvc_impl::rx_polyphase_cvc_impl(
        std::vector<float> &taps, int L) :
        gr::sync_decimator("rx_polyphase_cvc",
                           gr::io_signature::make(1, 1, sizeof(gr_complex)),
                           gr::io_signature::make(1, 1, sizeof(gr_complex) * L),
                           L / 2),
                           rx_polyphase_kernel(taps, L),
                           d_L(L)
    {
      if(d_L < 2 || d_L % 2 != 0){
        throw std::runtime_error("L has to be even and >= 2!");
      }

      // history is needed to generate type-III polyphase components
      set_history(L);
    }

    /*
     * Our virtual destructor.
     */
    rx_polyphase_cvc_impl::~rx_polyphase_cvc_impl()
    {
    }

    int
    rx_polyphase_cvc_impl::work(int noutput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int nout = rx_polyphase_kernel::generic_work(out, in, noutput_items);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace fbmc */
} /* namespace gr */

