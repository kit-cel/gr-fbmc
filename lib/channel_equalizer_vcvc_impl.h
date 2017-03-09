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

#ifndef INCLUDED_FBMC_CHANNEL_EQUALIZER_VCVC_IMPL_H
#define INCLUDED_FBMC_CHANNEL_EQUALIZER_VCVC_IMPL_H

#include <fbmc/channel_equalizer_vcvc.h>

namespace gr {
  namespace fbmc {

    class channel_equalizer_vcvc_impl : public channel_equalizer_vcvc {
    private:
      int d_frame_len, d_pilot_timestep, d_subcarriers, d_o, d_bands;
      std::vector<int> d_pilot_carriers;
      std::vector<float> d_taps;
      float d_pilot_amp;
      //std::vector<gr_complex> d_R;
			gr_complex* d_despread_temp;
      //void despread(gr_complex* out, int noutput_items);

    public:
      channel_equalizer_vcvc_impl(int frame_len, int overlap, int bands, int pilot_timestep, std::vector<int> pilot_carriers,
                                  int subcarriers, std::vector<float> taps, float pilot_amplitude);

      ~channel_equalizer_vcvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CHANNEL_EQUALIZER_VCVC_IMPL_H */
