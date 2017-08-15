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

#ifndef INCLUDED_FBMC_TX_DUMMY_MIXER_CC_IMPL_H
#define INCLUDED_FBMC_TX_DUMMY_MIXER_CC_IMPL_H

#include <fbmc/tx_dummy_mixer_cc.h>

namespace gr {
  namespace fbmc {

    class tx_dummy_mixer_cc_impl : public tx_dummy_mixer_cc
    {
     private:
      int d_bands, d_subcarriers, d_symbols, d_curr_band, d_max, d_counter;
      std::vector<gr_complex> d_phase_inc;
      float d_samp_rate;
      gr_complex d_phase;
      double d_bw;
      gr_complex get_freq(int band);

     public:
      tx_dummy_mixer_cc_impl(int bands, double bandwidth, int symbols, int subcarriers, float samp_rate);
      ~tx_dummy_mixer_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_TX_DUMMY_MIXER_CC_IMPL_H */

