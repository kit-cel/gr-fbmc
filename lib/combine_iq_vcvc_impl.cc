/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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
#include "combine_iq_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    combine_iq_vcvc::sptr
    combine_iq_vcvc::make(int L)
    {
      return gnuradio::get_initial_sptr
        (new combine_iq_vcvc_impl(L));
    }

    /*
     * The private constructor
     */
    combine_iq_vcvc_impl::combine_iq_vcvc_impl(int L)
      : gr::sync_decimator("combine_iq_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), 2),
              d_L(L)
    {
      set_output_multiple(L);
    }

    /*
     * Our virtual destructor.
     */
    combine_iq_vcvc_impl::~combine_iq_vcvc_impl()
    {
    }

    int
    combine_iq_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        if(noutput_items < d_L)
          throw std::runtime_error("Output buffer too small");

        for(int l = 0; l < d_L; l++)
            out[l] = gr_complex(in[l].real(), in[l+d_L].real());

        // Tell runtime system how many output items we produced.
        return d_L;
    }

  } /* namespace fbmc */
} /* namespace gr */

