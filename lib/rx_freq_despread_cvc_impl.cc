/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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
#include "rx_freq_despread_cvc_impl.h"

namespace gr {
  namespace fbmc {

    rx_freq_despread_cvc::sptr
    rx_freq_despread_cvc::make(std::vector<float> taps, int subcarriers, float pilot_amplitude, int pilot_timestep, std::vector<int> pilot_carriers)
    {
      return gnuradio::get_initial_sptr
        (new rx_freq_despread_cvc_impl(taps, subcarriers, pilot_amplitude, pilot_timestep, pilot_carriers));
    }

    /*
     * The private constructor
     */
    rx_freq_despread_cvc_impl::rx_freq_despread_cvc_impl(std::vector<float> taps, int subcarriers, float pilot_amplitude, int pilot_timestep, std::vector<int> pilot_carriers)
      : gr::block("rx_freq_despread_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * subcarriers)),
    d_subcarriers(subcarriers)
    {}

    /*
     * Our virtual destructor.
     */
    rx_freq_despread_cvc_impl::~rx_freq_despread_cvc_impl()
    {
    }

    void
    rx_freq_despread_cvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items * d_subcarriers;
    }

    int
    rx_freq_despread_cvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

