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
#include "frame_generator_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    frame_generator_vcvc::sptr
    frame_generator_vcvc::make(int sym_len, int frame_len)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_vcvc_impl(sym_len, frame_len));
    }

    /*
     * The private constructor
     */
    frame_generator_vcvc_impl::frame_generator_vcvc_impl(int sym_len, int frame_len)
      : gr::block("frame_generator_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len)),
              d_sym_len(sym_len),
              d_frame_len(frame_len),
              d_overlap(4),
              d_num_payload_sym(0),
              d_payload_sym_ctr(0)
    {
    	// the frame length has to be a multiple of 4 because of the periodicity beta matrix
    	if(d_frame_len % 4 != 0)
    		throw std::runtime_error(std::string("frame length must be a a multiple of 4 because of the periodicity beta matrix"));
    	// this is necessary to convey at least 1 payload symbol per frame
    	if(d_frame_len <= d_overlap)
    		throw std::runtime_error(std::string("frame length must be greater than the overlap"));
    	d_num_payload_sym = d_frame_len - d_overlap;

    	// the block's output buffer must be able to hold at least one symbol + the zero symbols
    	set_min_output_buffer(sizeof(gr_complex)*sym_len*(d_overlap+1));
    }

    /*
     * Our virtual destructor.
     */
    frame_generator_vcvc_impl::~frame_generator_vcvc_impl()
    {
    }

    void
    frame_generator_vcvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = noutput_items;
    }

    int
    frame_generator_vcvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (1);
        // Insert zero symbols if the end of the frame is reached
        if(d_payload_sym_ctr == d_num_payload_sym - 1) // the current input buffer contains the last payload symbol in the frame
        {
        	// copy input buffer to output
        	memcpy(out, in, sizeof(gr_complex)*d_sym_len);

        	// append zero symbols
        	memset(out+d_sym_len, 0, sizeof(gr_complex)*d_sym_len*d_overlap);
        	noutput_items = 1 + d_overlap;
        	d_payload_sym_ctr = 0; // reset counter
        }
        else
        {
        	// copy input buffer to output
        	memcpy(out, in, sizeof(gr_complex)*d_sym_len);
        	noutput_items = 1;
        	d_payload_sym_ctr++;
        }

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

