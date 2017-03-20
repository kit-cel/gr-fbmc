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

#ifndef INCLUDED_FBMC_CAZAC_FREQ_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_CAZAC_FREQ_SYNC_CC_IMPL_H

#include <fbmc/cazac_freq_sync_cc.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace fbmc {

    class cazac_freq_sync_cc_impl : public cazac_freq_sync_cc
    {
     private:
      int d_bands, d_subcarriers, d_frame_len, d_curr_samp, d_fft_len;
      float d_fo;
      int d_range;
      std::vector<gr_complex> d_fft_sequences;
      gr::fft::fft_complex* d_fft;
      gr_complex d_temp, d_phase, d_phase_inc;

      float get_freq_offset(gr_complex* in);

     public:
      cazac_freq_sync_cc_impl(int subcarriers, int bands, int frame_len, int fft_size, std::vector<gr_complex> fft_sequences);
      ~cazac_freq_sync_cc_impl();
      float get_fo();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CAZAC_FREQ_SYNC_CC_IMPL_H */

