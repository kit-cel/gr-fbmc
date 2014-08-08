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
    preamble_insertion_vcvc::make(int L, int frame_len, int overlap, std::vector<int> channel_map, std::vector<gr_complex> prbs)
    {
      return gnuradio::get_initial_sptr
        (new preamble_insertion_vcvc_impl(L, frame_len, overlap, channel_map, prbs));
    }

    /*
     * The private constructor
     */
    preamble_insertion_vcvc_impl::preamble_insertion_vcvc_impl(int L, int frame_len, int overlap, std::vector<int> channel_map, std::vector<gr_complex> prbs)
      : gr::sync_block("preamble_insertion_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
              d_L(L),
              d_frame_len(frame_len),
              d_ctr(0),
              d_overlap(overlap),
              d_channel_map(channel_map),
              d_prbs(prbs)
    {}

    /*
     * Our virtual destructor.
     */
    preamble_insertion_vcvc_impl::~preamble_insertion_vcvc_impl()
    {}

    int
    preamble_insertion_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      if(d_ctr == 0 || d_ctr == d_overlap) // insert PRBS
  		{
  			// insert PRBS
  			int n=0;
        for(int i=0; i<d_L; i++)
        {
          if(d_channel_map[i]!=0)
            out[i] = d_prbs[n++];
          else
            out[i] = gr_complex(0,0);
        }
  		}
  		else // just copy the input to the output
  		  memcpy(out, in, sizeof(gr_complex)*d_L);
		
		  d_ctr = (d_ctr + 1) % d_frame_len;

      // Tell runtime system how many output items we produced.
      return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

