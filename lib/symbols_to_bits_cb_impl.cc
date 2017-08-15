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
#include "symbols_to_bits_cb_impl.h"

namespace gr {
  namespace fbmc {

    symbols_to_bits_cb::sptr
    symbols_to_bits_cb::make(gr::digital::constellation_sptr constellation)
    {
      return gnuradio::get_initial_sptr
        (new symbols_to_bits_cb_impl(constellation));
    }

    /*
     * The private constructor
     */
    symbols_to_bits_cb_impl::symbols_to_bits_cb_impl(gr::digital::constellation_sptr constellation)
      : gr::sync_block("symbols_to_bits_cb",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(char)))
    {
      d_const = constellation;
    }

    /*
     * Our virtual destructor.
     */
    symbols_to_bits_cb_impl::~symbols_to_bits_cb_impl()
    {
    }

    int
    symbols_to_bits_cb_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        char *out = (char *) output_items[0];

        // Map the input samples to the nearest constellation points and return their values
        for(int i = 0; i < noutput_items; i++)
            out[i] = static_cast<char>(d_const->decision_maker(&in[i])); // This cast should be fine for constellations with up to 256 points

        // Tell runtime system how many output items we produced.
        //std::cout << "symbols to bits returned: " << noutput_items << std::endl;
        
        // FIXME: This block actually does not return bits but integers in the range of [ 0, size(const) )
        return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

