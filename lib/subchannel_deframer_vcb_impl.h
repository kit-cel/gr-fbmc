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

#ifndef INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_IMPL_H
#define INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_IMPL_H

#include <fbmc/subchannel_deframer_vcb.h>

namespace gr {
  namespace fbmc {

    class subchannel_deframer_vcb_impl : public subchannel_deframer_vcb
    {
     private:
      std::vector<gr_complex> d_zc_sequence;
      std::vector<int> d_pilot_carriers;
      std::vector<std::vector<gr_complex> > d_current_frame;
      int d_pilot_timestep, d_fft_len, d_num_chan, d_num_symbols;
      float d_min_corr;

      int process_one_subband(int band);

     public:
      subchannel_deframer_vcb_impl(std::vector<gr_complex>& zc_sequence, float min_corr, int fft_len, int num_chan,
                                   int num_symbols, int pilot_timestep, std::vector<int>& pilot_carriers);
      ~subchannel_deframer_vcb_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_IMPL_H */

