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
#include "apply_betas_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    apply_betas_vcvc::sptr
    apply_betas_vcvc::make(int L)
    {
      return gnuradio::get_initial_sptr
        (new apply_betas_vcvc_impl(L));
    }

    /*
     * The private constructor
     */
    apply_betas_vcvc_impl::apply_betas_vcvc_impl(int L)
      : gr::sync_block("apply_betas_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
				d_L(L),
				d_beta(NULL),
				d_sym_ctr(0)
        {
        	set_relative_rate(1.0); // make this block behave like a sync block

        	// calculate minimum set of betas
        	// frames that exceed the dimension of this set can be processed by periodic continuation of this matrix
        	d_beta = new gr_complex*[4]; // allocate memory for beta matrix
        	for(int i = 0; i < 4; i++)
        		d_beta[i] = new gr_complex[2];

        	for(int l = 0; l < 4; l++)
        	{
        		for(int k = 0; k < 2; k++)
        		{
        			gr_complex fac1, fac2;
        			if( (l*k) % 2) // if m*k odd
        				fac1 = -1;
        			else
        				fac1 = 1;
        			if( (l+k) % 2) // if m+k odd
        				fac2 = gr_complex(0,1);
        			else
        				fac2 = 1;
        			d_beta[l][k] = fac1*fac2;
        		}
        	}
        }
    /*
     * Our virtual destructor.
     */
    apply_betas_vcvc_impl::~apply_betas_vcvc_impl()
    {
    	delete[] d_beta;
    }

    int
    apply_betas_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Apply the betas to the current symbol
        for(int l = 0; l < d_L; l++)
        	out[l] = d_beta[l%4][d_sym_ctr] * in[l];

        // update symbol counter
        d_sym_ctr = (d_sym_ctr+1) % 2;

        // Return one vector of L elements
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

