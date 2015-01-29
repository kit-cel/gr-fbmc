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


#ifndef INCLUDED_FBMC_IOTA_FILTERBANK__RX_KERNEL_H
#define INCLUDED_FBMC_IOTA_FILTERBANK__RX_KERNEL_H

#include <fbmc/api.h>
#include <gnuradio/filter/filterbank.h>
#include <fftw3.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief actual kernel for SMT Filterbank
     *
     */
    class FBMC_API rx_polyphase_kernel: public filter::kernel::filterbank
    {
    public:
      rx_polyphase_kernel(std::vector<float> &taps, int L);
      ~rx_polyphase_kernel();

      int generic_work(gr_complex* out, const gr_complex* in, int noutput_items);

      int L(){return d_L;};
      int overlap(){return d_overlap;};
      int fft_size(){return d_L;};
      std::vector<float> taps(){return d_taps;};
      std::vector<std::vector<float> > filterbank_taps(){return filterbank::taps();};

    private:
      int d_L;
      int d_overlap;
      std::vector<float> d_taps;

      gr_complex* d_fft_in_buf;
      gr_complex* d_fft_out_buf;
      fftwf_plan d_fft_plan; // see gr-fft files for FFTW plan usage.
      std::vector<std::vector<float> > d_prototype_taps;
      void set_taps(std::vector<float> &taps);
      std::vector<gr_complex*> d_buffers;
      void initialize_branch_buffers(int L, int ntaps);
      inline gr_complex filter_branch(gr_complex in_sample, int branch);
      inline void update_branch_buffer(gr_complex in_sample, int branch);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_RX_POLYPHASE_KERNEL_H */

