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
#include "serial_to_parallel_cvc_impl.h"

namespace gr {
  namespace fbmc {

    serial_to_parallel_cvc::sptr
    serial_to_parallel_cvc::make(int len_in, int vlen_out, std::vector<int> channel_map)
    {
      return gnuradio::get_initial_sptr
        (new serial_to_parallel_cvc_impl(len_in, vlen_out, channel_map));
    }

    /*
     * The private constructor
     */
    serial_to_parallel_cvc_impl::serial_to_parallel_cvc_impl(int len_in, int vlen_out, std::vector<int> channel_map)
      : gr::sync_decimator("serial_to_parallel_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*vlen_out), len_in),
              d_len_in(len_in),
              d_vlen_out(vlen_out),
              d_channel_map(channel_map)
    {
    	if(d_len_in > d_vlen_out)
        throw std::runtime_error(std::string("Output vector length must at least be equal to the input length"));
      else if(d_channel_map.size() != d_vlen_out)
        throw std::runtime_error(std::string("Channel map does not match the output vector length"));
    }

    /*
     * Our virtual destructor.
     */
    serial_to_parallel_cvc_impl::~serial_to_parallel_cvc_impl()
    {
    }

    int
    serial_to_parallel_cvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        gr_complex *in = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        // convert sample stream to vector, use channel map
        for(int i=0; i < d_vlen_out; i++)
        {
          if(d_channel_map[i] == 0)
            *out++ = gr_complex(0,0);
          else
            *out++ = *in++;
        }

        // Tell runtime system how many output items we produced.
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

