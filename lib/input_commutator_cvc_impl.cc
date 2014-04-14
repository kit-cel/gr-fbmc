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
#include "input_commutator_cvc_impl.h"

namespace gr {
  namespace fbmc {

    input_commutator_cvc::sptr
    input_commutator_cvc::make(int L)
    {
      return gnuradio::get_initial_sptr
        (new input_commutator_cvc_impl(L));
    }

    /*
     * The private constructor
     */
    input_commutator_cvc_impl::input_commutator_cvc_impl(int L)
      : gr::sync_decimator("input_commutator_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L), L/2),
              d_L(L)
    {
        if(d_L < 2 || d_L % 2 != 0)
            throw std::runtime_error("L has to be even and >= 2!");
            
        // set the sample buffer up
        d_buf.resize(2*d_L);
        memset(&d_buf[0], 0, d_buf.size()*sizeof(gr_complex));
    }

    /*
     * Our virtual destructor.
     */
    input_commutator_cvc_impl::~input_commutator_cvc_impl()
    {
    }

    int
    input_commutator_cvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        gr_complex *in = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // write input into internal buffer with an offset of L/2-1 samples
        // the offset is needed to generate type-III polyphase components
        memcpy(&d_buf[0]+d_L/2-1, in, d_L*sizeof(gr_complex));  
        
        // one call to work produces 2 output vectors
        // the second half of each vector is equal to the first half
        for(int k = 0; k < 2; k++)
        {
            for(int l = 0; l < d_L/2; l++)
            {
                out[k*d_L + l] = d_buf[k*d_L/2 + d_L/2-1-l];
                out[k*d_L + d_L/2 + l] = out[k*d_L + l];
            }
        }
        
        // shift the L leftmost samples out of the buffer
        d_buf = d_buf.shift(d_L); 

        // Tell runtime system how many output items we produced.
        return 2;
    }

  } /* namespace fbmc */
} /* namespace gr */

