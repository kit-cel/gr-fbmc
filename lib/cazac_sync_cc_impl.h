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

#ifndef INCLUDED_FBMC_CAZAC_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_CAZAC_SYNC_CC_IMPL_H

#include <fbmc/cazac_sync_cc.h>
#include <gnuradio/fft/fft.h>
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    class cazac_sync_cc_impl : public cazac_sync_cc {
    private:
      int d_frame_len, d_subcarriers, d_bands, d_o, d_curr_sample, d_fft_len;
      float d_threshold;
      bool d_synced;
      std::vector<std::vector<gr_complex> > d_zc_seqs;
      std::vector<gr_complex> d_zc_fft;
      fft::fft_complex *d_fft;
      lv_32fc_t d_phase, d_phase_increment;

      void zero_pad(std::vector<gr_complex> *vector, int len);

      void circshift(std::vector<gr_complex> *vector, int shift);

      int time_sync(const gr_complex *in);

      void freq_sync(const gr_complex *out);
      void freq_correction(gr_complex *out, int length);

    public:
      cazac_sync_cc_impl(int subcarriers, int bands, int overlap, int frame_len, float threshold,
                         std::vector<std::vector<gr_complex> > zc_seqs, std::vector<gr_complex> zc_fft);

      ~cazac_sync_cc_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CAZAC_SYNC_CC_IMPL_H */

