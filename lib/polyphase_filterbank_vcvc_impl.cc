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
#include "polyphase_filterbank_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    polyphase_filterbank_vcvc::sptr
    polyphase_filterbank_vcvc::make(std::vector<gr_complex> taps, int L)
    {
      return gnuradio::get_initial_sptr
        (new polyphase_filterbank_vcvc_impl(taps, L));
    }

    /*
     * The private constructor
     */
    polyphase_filterbank_vcvc_impl::polyphase_filterbank_vcvc_impl(std::vector<gr_complex> taps, int L)
      : gr::sync_block("polyphase_filterbank_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
              d_prototype_taps(taps),
              d_L(L),
              d_num_branch_taps(0),
              d_branch_taps(NULL),
              d_group_delay((taps.size()-1)/2)
    {
    	// prepare the filter branches
    	if(d_prototype_taps.size() % d_L != 0)
    		std::cerr << "polyphase_filterbank_vcvc_impl::polyphase_filterbank_vcvc_impl(): Prototype filter truncated by " << d_prototype_taps.size() % d_L << " taps." << std::endl;
    	d_num_branch_taps = (d_prototype_taps.size()/d_L - 1)*2; // NOTE: The filter might be cropped by this
    	d_branch_taps = new gr_complex*[d_L];
    	for(int i = 0; i < d_L; i++)
    	{
    		d_branch_taps[i] = new gr_complex[d_num_branch_taps];
    		// write the upsampled prototype coefficients into the branch filter
    		memset(d_branch_taps[i], 0, d_num_branch_taps*sizeof(gr_complex));
    		//FIXME write taps into filters
    	}

    	for(int l = 0; l < d_L; l++)
    	{
    		std::cout << "l: " << l << "taps:\t";
    		for(int n = 0; n < d_num_branch_taps; n++)
    			std::cout << d_branch_taps[l][n] << "\t";
    		std::cout << std::endl;
    	}

    }

    /*
     * Our virtual destructor.
     */
    polyphase_filterbank_vcvc_impl::~polyphase_filterbank_vcvc_impl()
    {
    	delete[] d_branch_taps;
    }

    int
    polyphase_filterbank_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Do <+signal processing+>

        // Tell runtime system how many output items we produced.
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

