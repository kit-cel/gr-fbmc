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


#ifndef INCLUDED_FBMC_RX_SDFT_KERNEL_H
#define INCLUDED_FBMC_RX_SDFT_KERNEL_H

#include <fbmc/api.h>
#include <fbmc/smt_kernel.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief PHYDYAS Filterbank RX Kernel
     *
     * \ingroup fbmc
     */
    class FBMC_API rx_sdft_kernel: public smt_kernel
    {
    public:
      rx_sdft_kernel(const std::vector<float> &taps, int L);
      ~rx_sdft_kernel();

      int generic_work(gr_complex* out, const gr_complex* in, int noutput_items);

      // using directives are only SWIG necessities
      using smt_kernel::L;
      using smt_kernel::overlap;
      using smt_kernel::taps;
      int fft_size(){return d_fft->inbuf_length();};

    protected:
      int get_noutput_items_for_ninput(int inbuf_size){return inbuf_size;};

    private:
      float* d_taps_al;
      gr_complex* d_multiply_res;
      gr_complex* d_add_res;
      gr::fft::fft_complex* d_fft;

      inline void multiply_with_taps(gr_complex *out_buff, const gr_complex *in_buff, int L, int overlap);
      inline void add_overlaps(gr_complex *out_buff, const gr_complex *in_buff, int L, int overlap);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_RX_SDFT_KERNEL_H */

