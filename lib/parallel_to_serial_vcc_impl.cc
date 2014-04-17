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
#include "parallel_to_serial_vcc_impl.h"

namespace gr {
  namespace fbmc {

    parallel_to_serial_vcc::sptr
    parallel_to_serial_vcc::make(int len_out, int vlen_in)
    {
      return gnuradio::get_initial_sptr
        (new parallel_to_serial_vcc_impl(len_out, vlen_in));
    }

    /*
     * The private constructor
     */
    parallel_to_serial_vcc_impl::parallel_to_serial_vcc_impl(int len_out, int vlen_in)
      : gr::sync_interpolator("parallel_to_serial_vcc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*vlen_in),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), (vlen_in-len_out)),
              d_len_out(len_out),
              d_vlen_in(vlen_in)
    {
        // validity checks
        if(d_vlen_in < 1 || d_len_out < 1 || d_len_out > d_vlen_in)
            throw std::runtime_error("Invalid parameters!");
    }

    /*
     * Our virtual destructor.
     */
    parallel_to_serial_vcc_impl::~parallel_to_serial_vcc_impl()
    {
    }

    int
    parallel_to_serial_vcc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Extract d_len_out samples out of the input vector, discard the rest (should be zeros anyway)
        memcpy(out, in, sizeof(gr_complex)*d_len_out);

        // Tell runtime system how many output items we produced.
        return d_len_out;
    }

  } /* namespace fbmc */
} /* namespace gr */

