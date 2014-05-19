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
    frame_generator_vcvc::make(int sym_len, int num_payload, int inverse, int num_overlap, int num_sync)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_vcvc_impl(sym_len, num_payload, inverse, num_overlap, num_sync));
    }

    /*
     * The private constructor
     */
    frame_generator_vcvc_impl::frame_generator_vcvc_impl(int sym_len, int num_payload, int inverse, int num_overlap, int num_sync)
      : gr::block("frame_generator_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len)),
              d_sym_len(sym_len),
              d_num_payload(num_payload),
              d_num_overlap(num_overlap),
              d_num_sync(num_sync),
              d_payload_sym_ctr(0),
              d_dropped_sym_ctr(0),
			  d_inverse(inverse)
    {
		// inverse has to be either 0 (insert zeros) or 1 (remove them)
		if (d_inverse != 0 && d_inverse != 1)
			throw std::runtime_error(std::string("inverse has to be either 0 or 1"));
			
		// at the moment, only an overlap of 4 is supported
		if(d_num_overlap != 4)
			throw std::runtime_error(std::string("overlap has to be 4"));
			
		// the number of payload symbols must be >= 1
		if (d_num_payload < 1)
			throw std::runtime_error(std::string("number of payload symbols must be > 0"));
			
    	// the frame length has to be a multiple of 4 because of the periodicity of the beta matrix
    	// the frame structure: ||num_sync|num_overlap|payload|num_overlap||...
    	d_num_frame = d_num_payload + d_num_overlap + d_num_sync + d_num_overlap;
    	if(d_num_frame % 4 != 0)
    		throw std::runtime_error(std::string("frame length must be a a multiple of 4 because of the periodicity beta matrix"));
		
		// the block's output buffer must be able to hold at least one symbol + the zero and sync symbols
    	set_output_multiple(d_num_sync + 2*d_num_overlap+1);
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
        ninput_items_required[0] = 1;
    }

    int
    frame_generator_vcvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        gr_complex *in = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];
        
        if(ninput_items[0] < 1)
			throw std::runtime_error(std::string("work was called but input buffer is empty"));
			
		if(noutput_items < d_num_overlap*2 + d_num_sync + 1)
			throw std::runtime_error(std::string("output buffer too small"));
        
        //std::cout << "enter frame generator " << d_inverse << std::endl;
        // The general frame structure is: || sync | zeros(overlap) | payload | zeros(overlap) || ...

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (1);
        noutput_items = 0; // add to this variable whenever symbols are inserted
		
		// check if we are inserting or removing zero/sync symbols
		if(d_inverse)
		{
			// remove zero symbols and overlap symbols if we are at the start of a frame
			if(d_payload_sym_ctr == 0)
			{
				int num_sym_to_drop = 2*d_num_overlap + d_num_sync; // this includes the whole preamble and the payload overlap
				if(d_dropped_sym_ctr < num_sym_to_drop) // there are still zero/sync symbols to drop
				{
					// drop the incoming symbol by doing nothing
					// increase dropped symbol counter
					d_dropped_sym_ctr += 1;
					// increase input buffer pointer by one symbol
					//in += d_sym_len;
					//std::cout << "drop zero/sync symbol number " << d_dropped_sym_ctr << std::endl;
				}
				else // all zero/sync symbols for this frame have been dropped, copy payload from input to output
				{
					// copy first payload symbol of frame to the output buffer
					//memcpy((void*) out, (void*) in, sizeof(gr_complex)*d_sym_len);
					for(int i = 0; i < d_sym_len; i++)
						*out++ = in[i];
					// increase payload symbol counter
					d_payload_sym_ctr += 1;
					// increase number of output items
					noutput_items += 1;
					//std::cout << "copy payload symbol number " << d_payload_sym_ctr << std::endl;
				}
			}
			else
			{
				// copy symbol to output and reset counter if needed
				//memcpy((void*) out, (void*) in, sizeof(gr_complex)*d_sym_len);
				for(int i = 0; i < d_sym_len; i++)
					*out++ = in[i];
				// increase payload symbol counter and wrap if needed
				d_payload_sym_ctr += 1;
				//std::cout << "copy payload symbol number " << d_payload_sym_ctr << std::endl;
				// increase output items
				noutput_items += 1;
				if (d_payload_sym_ctr == d_num_payload)
				{
					d_payload_sym_ctr = 0;
					// reset counter for dropped symbols
					d_dropped_sym_ctr = 0;
				}
			}
		}
		else
		{
			// If we are at the start of the frame, insert placeholder symbols for a preamble
			if(d_payload_sym_ctr == 0)
			{
				// insert preamble and num_overlap zero symbols
				//memset((void*) out, 0, sizeof(gr_complex)*d_sym_len*(d_num_sync+d_num_overlap));
				for(int i = 0; i < d_sym_len*(d_num_overlap+d_num_sync); i++)
					*out++ = 0;
				// shift output pointer
				//out += d_sym_len*(d_num_sync+d_num_overlap);
				// increase output items
				noutput_items += (d_num_sync + d_num_overlap);
				//std::cout << "start of frame, insert zeros" << std::endl;
			}
			
			// insert the payload symbol
			//memcpy((void*) out, (void*) in, sizeof(gr_complex)*d_sym_len);
			for(int i = 0; i < d_sym_len; i++)
				*out++ = in[i];
			// shift output pointer
			//out += d_sym_len;
			// increase output items
			noutput_items += 1;
			// increase payload symbol counter
			d_payload_sym_ctr += 1;
			//std::cout << "copy payload symbol number: " << d_payload_sym_ctr << std::endl;
			
			// If we are at the end, insert num_overlap zero symbols to allow for filter settling
			if(d_payload_sym_ctr == d_num_payload) // the current input buffer contains the last payload symbol in the frame
			{
				// append zero symbols
				//memset((void*) out, 0, sizeof(gr_complex)*d_sym_len*d_num_overlap);
				for(int i = 0; i < d_sym_len*d_num_overlap; i++)
					*out++ = 0;
				// increase output items
				noutput_items += d_num_overlap;
				// reset counter
				d_payload_sym_ctr = 0; 
				//std::cout << "end of frame, insert zeros" << std::endl;
			}
		}

        // Tell runtime system how many output items we produced.
        //std::cout << "frame_generator_vcvc " << d_inverse << " returned: " << noutput_items << std::endl;
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

