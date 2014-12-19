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

#ifndef INCLUDED_FBMC_TX_SDFT_KERNEL_H
#define INCLUDED_FBMC_TX_SDFT_KERNEL_H

#include <fbmc/api.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief TX SDFT implementation of an IFFT -> PFB -> Commutator
     *
     */
    class FBMC_API tx_sdft_kernel
    {
    public:
      tx_sdft_kernel(std::vector<float> taps, int L);
      ~tx_sdft_kernel();

      int
      generic_work(gr_complex* out, const gr_complex* in, int noutput_items);

      int L(){return d_L;};
      int overlap(){return d_overlap;};
      std::vector<float> taps(){return d_taps;};

    private:
      int d_L;
      int d_overlap;
      std::vector<float> d_taps;
      float* d_taps_al;

      gr::fft::fft_complex* d_fft;

      gr_complex* d_multiply_res;
      inline void
      multiply_with_taps(gr_complex* out, const gr_complex* in);

      gr_complex* d_add_buf;
      inline int
      calculate_sum_result(gr_complex* outbuf, const gr_complex* res);

      inline int
      process_one_vector(gr_complex* outbuf, const gr_complex* inbuf);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_TX_SDFT_KERNEL_H */

