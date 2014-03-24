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
#include "serialize_iq_cc_impl.h"

namespace gr {
  namespace fbmc {

    serialize_iq_cc::sptr
    serialize_iq_cc::make()
    {
      return gnuradio::get_initial_sptr
        (new serialize_iq_cc_impl());
    }

    /*
     * The private constructor
     */
    serialize_iq_cc_impl::serialize_iq_cc_impl()
      : gr::sync_interpolator("serialize_iq_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), 2)
    {}

    /*
     * Our virtual destructor.
     */
    serialize_iq_cc_impl::~serialize_iq_cc_impl()
    {
    }

    int
    serialize_iq_cc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Split complex numbers up into real and imaginary part and serialize them (format: IQIQIQ...)
        for(int i = 0; i < noutput_items; i++)
        {
        	out[2*i] = in[i].real();
        	out[2*i+1] = in[i].imag();
        }

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

