/* -*- c++ -*- */
/* 
 * Copyright 2014  Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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
#include "apply_betas_cc_impl.h"

#include <iostream>

namespace gr {
  namespace fbmc {

    apply_betas_cc::sptr
    apply_betas_cc::make(int K, int M)
    {
      return gnuradio::get_initial_sptr
        (new apply_betas_cc_impl(K, M));
    }

    /*
     * The private constructor
     */
    apply_betas_cc_impl::apply_betas_cc_impl(int K, int M)
      : gr::tagged_stream_block("apply_betas_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), "frame_len"),
              d_K(K),
              d_M(M),
              d_beta(NULL)
    {
    	set_relative_rate(1.0); // make this block behave like a sync block

    	// calculate minimum set of betas
    	// frames that exceed the dimension of this set can be processed by periodic continuation of this matrix
    	d_beta = new gr_complex*[4]; // allocate memory for beta matrix
    	for(int i = 0; i < 4; i++)
    		d_beta[i] = new gr_complex[2];

    	for(int m = 0; m < 4; m++)
    	{
    		for(int k = 0; k < 2; k++)
    		{
    			gr_complex fac1, fac2;
    			if( (m*k) % 2) // if m*k odd
    				fac1 = -1;
    			else
    				fac1 = 1;
    			if( (m+k) % 2) // if m+k odd
    				fac2 = gr_complex(0,1);
    			else
    				fac2 = 1;
    			d_beta[m][k] = fac1*fac2;
    		}
    	}
    }

    /*
     * Our virtual destructor.
     */
    apply_betas_cc_impl::~apply_betas_cc_impl()
    {
    	delete[] d_beta;
    }

    int
    apply_betas_cc_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0];
      std::cout << "noutput_items = " << noutput_items << std::endl;
      return noutput_items ;
    }

    int
    apply_betas_cc_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // check if the input has the correct size
        assert(d_K*d_M == ninput_items[0]);

        // Apply the betas to the respective symbols
        // This should always work on an entire frame as d_M*d_K == frame_len
        // TODO: check if d_M*d_K == frame_len
        for(int m = 0; m < d_M; m++)
        {
        	for(int k = 0; k < d_K; k++)
        	{
        		*out++ = d_beta[m%4][k%2] * (*in++);
        	}
        }

        // Tell runtime system how many output items we produced.
        std::cout << "return noutput_items from work = " << noutput_items << std::endl;
        return ninput_items[0];
    }

  } /* namespace fbmc */
} /* namespace gr */

