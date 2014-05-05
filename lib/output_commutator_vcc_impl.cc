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
#include "output_commutator_vcc_impl.h"

namespace gr {
  namespace fbmc {

    output_commutator_vcc::sptr
    output_commutator_vcc::make(int L)
    {
      return gnuradio::get_initial_sptr
        (new output_commutator_vcc_impl(L));
    }

    /*
     * The private constructor
     */
    output_commutator_vcc_impl::output_commutator_vcc_impl(int L)
      : gr::sync_interpolator("output_commutator_vcc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), L/2),
              d_L(L)
    {
    	assert(d_L % 2 == 0); // vector length must be even
    }

    /*
     * Our virtual destructor.
     */
    output_commutator_vcc_impl::~output_commutator_vcc_impl()
    {
    }

    int
    output_commutator_vcc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Add first and second half and write the result to the output buffer
        for(int l = 0; l < d_L/2; l++)
        	out[l] = in[l] + in[l+d_L/2];

        // Tell runtime system how many output items we produced.
        //std::cout << "output commutator returned: " << d_L/2 << std::endl;
        return d_L/2;
    }

  } /* namespace fbmc */
} /* namespace gr */

