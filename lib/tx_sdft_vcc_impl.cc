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
#include "tx_sdft_vcc_impl.h"

namespace gr {
  namespace fbmc {

    tx_sdft_vcc::sptr
    tx_sdft_vcc::make(std::vector<float> taps, int L)
    {
      return gnuradio::get_initial_sptr(new tx_sdft_vcc_impl(taps, L));
    }

    /*
     * The private constructor
     */
    tx_sdft_vcc_impl::tx_sdft_vcc_impl(std::vector<float> taps, int L) :
        gr::sync_interpolator(
            "tx_sdft_vcc", gr::io_signature::make(1, 1, sizeof(gr_complex) * L),
            gr::io_signature::make(1, 1, sizeof(gr_complex)),
            L / 2 /*also sets output_multiple*/), tx_sdft_kernel(taps, L)
    {
      // this is awkward. for streams just use num filtertaps
      // for vectors do a +1. easy, hu?
      // also remember input has double datarate due to this combine IQ stage.
//      set_history((overlap() / 2) + 1);
    }

    /*
     * Our virtual destructor.
     */
    tx_sdft_vcc_impl::~tx_sdft_vcc_impl()
    {
    }

    int
    tx_sdft_vcc_impl::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int nout = tx_sdft_kernel::generic_work(out, in, noutput_items);

      // Tell runtime system how many output items we produced.
      //std::cout << "tx sdft return " << nout << std::endl;
      return nout;
    }

  } /* namespace fbmc */
} /* namespace gr */

