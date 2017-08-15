/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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
#include <fbmc/smt_kernel.h>

#include <iostream>

namespace gr {
  namespace fbmc {

    smt_kernel::smt_kernel(const std::vector<float> &taps, int L) :
        d_taps(taps), d_L(L), d_overlap(0)
    {
    }

    smt_kernel::~smt_kernel()
    {
    }

    std::vector<gr_complex>
    gr::fbmc::smt_kernel::generic_work_python(const std::vector<gr_complex>& inbuf)
    {
      std::cout << "smt_kernel::generic_work_python\n";
      std::vector<gr_complex> outbuf;
      if(inbuf.size() < fft_size()) {
        std::cout << "Not enough input items to calculate at least 1 FFT!\n";
        return outbuf;
      }
      if(inbuf.size() % (L() / 2) != 0){
        std::cout << "input - output ration not met!\n";
        return outbuf;
      }

      int noutput_items = get_noutput_items_for_ninput(inbuf.size());
      outbuf.resize(noutput_items * L());

      std::cout << "requested items = " << noutput_items << ", inbuf.size() = " << inbuf.size()
          << std::endl;

//      // fancy new shit! Using data() member on vectors. C++11.
      int nout = generic_work(outbuf.data(), inbuf.data(), noutput_items);
      return outbuf;
    }

  } /* namespace fbmc */
} /* namespace gr */


