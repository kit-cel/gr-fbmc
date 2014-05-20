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
#include "preamble_insertion_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    preamble_insertion_vcvc::sptr
    preamble_insertion_vcvc::make(int L, int frame_len, std::string type, int overlap)
    {
      return gnuradio::get_initial_sptr
        (new preamble_insertion_vcvc_impl(L, frame_len, type, overlap));
    }

    /*
     * The private constructor
     */
    preamble_insertion_vcvc_impl::preamble_insertion_vcvc_impl(int L, int frame_len, std::string type, int overlap)
      : gr::sync_block("preamble_insertion_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
              d_L(L),
              d_frame_len(frame_len),
              d_type(type), 
              d_ctr(0),
              d_overlap(overlap),
              d_num_equal_sym(2)
    {
		// the preamble follows roughly the IAM2 method as described in 
		// "Channel estimation methods for preamble-based OFDM/OQAM modulations (Lele, Javaudin, Siohan)"
		
		// the original method is modified in a a way that it repeats the pilot symbol a number of times
		// to also allow timing synchronization and carrier frequency offset correction
		
		d_num_preamble_sym = d_num_equal_sym + d_overlap;
		
		// some asserts
		if(d_type != "IAM2")
			throw std::runtime_error(std::string("Only IAM2 is implemented and acceptable as type string"));
		if(d_frame_len < d_num_equal_sym + d_overlap + 1 + d_overlap)
			throw std::runtime_error(std::string("The frame can not be shorter than the preamble"));
		if(d_L % 4 != 0)
			throw std::runtime_error(std::string("Number of subcarriers must be a multiple of 4"));
		if(d_overlap != 4)
			throw std::runtime_error(std::string("Overlap must be 4"));
		if(d_num_equal_sym < 2)
			throw std::runtime_error(std::string("At least 2 identical symbols are required for a correlation"));
	}

    /*
     * Our virtual destructor.
     */
    preamble_insertion_vcvc_impl::~preamble_insertion_vcvc_impl()
    {
    }

    int
    preamble_insertion_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        if(d_ctr < d_num_equal_sym+d_overlap) // insert IAM2 pilot
		{
			// the sync sequence consists of the periodic continuation of [1 1 -1 -1]
			for(int i = 0; i < d_L; i++)
			{
				if( i%4 == 0 || i%4 == 1 )
					out[i] = 1;
				else
					out[i]= -1;
			}			
		}
		else // just copy the input to the output
		{
			for(int i = 0; i < d_L; i++)
				out[i] = in[i];
		}
		
		d_ctr = (d_ctr + 1) % d_frame_len;

        // Tell runtime system how many output items we produced.
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

