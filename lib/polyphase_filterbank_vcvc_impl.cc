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
    polyphase_filterbank_vcvc::make(int L, std::vector<gr_complex> prototype_taps)
    {
      return gnuradio::get_initial_sptr
        (new polyphase_filterbank_vcvc_impl(L, prototype_taps));
    }

    /*
     * The private constructor
     */
    polyphase_filterbank_vcvc_impl::polyphase_filterbank_vcvc_impl(int L, std::vector<gr_complex> prototype_taps)
      : gr::sync_block("polyphase_filterbank_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
              d_prototype_taps(prototype_taps),
              d_L(L),
              d_num_branch_taps(0),
              d_branch_taps(NULL),
              d_branch_states(NULL),
              d_group_delay(0)
    {
    	d_group_delay = (d_prototype_taps.size() - 1)/2;

    	// pad the prototype to an integer multiple length of L
    	while(d_prototype_taps.size() % d_L != 0)
    		d_prototype_taps.push_back(gr_complex(0,0));

    	// prepare the filter branches
    	d_num_branch_taps = d_prototype_taps.size()/d_L*2; // calculate number of taps per branch filter
    	d_branch_taps = new gr_complex*[d_L];
    	d_branch_states = new boost::circular_buffer<gr_complex>[d_L];
    	for(int l = 0; l < d_L; l++)
    	{
    		// write the upsampled prototype coefficients into the branch filter
    		d_branch_taps[l] = new gr_complex[d_num_branch_taps];
    		memset((void*) d_branch_taps[l], 0, d_num_branch_taps*sizeof(gr_complex));
    		int offset = 0;
    		if(l >= L/2)
    			offset = 1;
    		for(int n = 0; n < d_num_branch_taps; n++)
    			if( (n+offset) % 2) // tap is zero due to oversampling
    				d_branch_taps[l][n] = 0;
    			else
    				d_branch_taps[l][n] = d_prototype_taps[l+(n/2)*d_L];

    		// set size of state registers and initialize them with zeros
    		d_branch_states[l].set_capacity(d_num_branch_taps); // this includes the new sample in each iteration
    		for(int i = 0; i < d_num_branch_taps; i++)
    			d_branch_states[l].push_front(gr_complex(0,0));
    	}
    }

    /*
     * Our virtual destructor.
     */
    polyphase_filterbank_vcvc_impl::~polyphase_filterbank_vcvc_impl()
    {
    	delete[] d_branch_taps;
    	delete[] d_branch_states;
    }

    void
    polyphase_filterbank_vcvc_impl::filter(gr_complex* in, gr_complex* out)
    {
    	for(int l = 0; l < d_L; l++)
    		filter_branch(&in[l], &out[l], l);
    }

    void
    polyphase_filterbank_vcvc_impl::filter_branch(gr_complex* in, gr_complex* out, int l)
    {
    	// the actual convolution
    	d_branch_states[l].push_front(*in);
    	*out = 0;
    	for(int n = 0; n < d_num_branch_taps; n++)
    		*out += d_branch_states[l][n] * d_branch_taps[l][n];
    }

    int
    polyphase_filterbank_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        gr_complex *in  = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        //FIXME: make sure center coeff has phase 0
//        if(d_group_delay % d_L != 0) // just a reminder to avoid unexpected behavior
//        {
//            std::cerr << "group delay: " << d_group_delay << ", L: " << d_L << std::endl;
//            throw std::runtime_error("assertion d_group_delay % d_L == 0 failed");
//        }

        // Filter one vector of L samples and return L samples
        filter(in, out);

        // Tell runtime system how many output items we produced.
        return 1;
    }

    std::vector<std::vector<gr_complex> >
    polyphase_filterbank_vcvc_impl::filter_branch_taps()
    {
      std::vector<std::vector<gr_complex> > tmp(d_L, std::vector<gr_complex>(d_num_branch_taps, 0));
      for(int l = 0; l < d_L; l++){
        for(int n = 0; n < d_num_branch_taps; n++){
          tmp[l][n] = d_branch_taps[l][n];
        }
      }
      return tmp;
    }

  } /* namespace fbmc */
} /* namespace gr */

