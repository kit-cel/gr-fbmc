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
      int d_subcarriers, d_bands, d_symbols, d_pilot_timestep, d_guard_carriers;
      float d_threshold;
      std::vector<gr_complex> d_preamble;
      std::vector<int> d_pilot_carriers;
      std::vector<bool> d_used_bands;

      std::vector<gr_complex> extract_preamble(int band);
      std::vector<std::vector<gr_complex> > d_curr_frame;
      void detect_used_bands();
      float correlate(const std::vector<gr_complex> &received);
      void extract_payload(char* out, unsigned int* bits_written);
      inline char demod(gr_complex sym, int iq);

     public:
      subchannel_deframer_vcb_impl(int subcarriers, int bands, int guard, float threshold,
                                   std::vector<gr_complex> preamble, int symbols, std::vector<int> pilot_carriers,
                                   int pilot_timestep);
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

