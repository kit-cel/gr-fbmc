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
#include "serial_to_parallel_cvc_impl.h"

namespace gr {
  namespace fbmc {

    serial_to_parallel_cvc::sptr
    serial_to_parallel_cvc::make(int len_in, int vlen_out)
    {
      return gnuradio::get_initial_sptr
        (new serial_to_parallel_cvc_impl(len_in, vlen_out));
    }

    /*
     * The private constructor
     */
    serial_to_parallel_cvc_impl::serial_to_parallel_cvc_impl(int len_in, int vlen_out)
      : gr::sync_decimator("serial_to_parallel_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*vlen_out), len_in),
              d_len_in(len_in),
              d_vlen_out(vlen_out)
    {
    	assert(d_len_in <= d_vlen_out);
    }

    /*
     * Our virtual destructor.
     */
    serial_to_parallel_cvc_impl::~serial_to_parallel_cvc_impl()
    {
    }

    int
    serial_to_parallel_cvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // convert sample stream to vector and pad if necessary
        memcpy(out, in, d_len_in*sizeof(gr_complex));
        if(d_len_in < d_vlen_out)
        	memset(out+d_len_in, 0, (d_vlen_out-d_len_in)*sizeof(gr_complex));

        // Tell runtime system how many output items we produced.
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

