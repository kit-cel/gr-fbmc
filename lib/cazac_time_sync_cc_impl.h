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

#ifndef INCLUDED_FBMC_CAZAC_TIME_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_CAZAC_TIME_SYNC_CC_IMPL_H

#include <fbmc/cazac_time_sync_cc.h>
#include <gnuradio/filter/fft_filter.h>
#include <gnuradio/filter/single_pole_iir.h>

namespace gr {
  namespace fbmc {

    class cazac_time_sync_cc_impl : public cazac_time_sync_cc
    {
     private:
      int d_frame_len;
      float d_threshold;
      std::vector<gr::filter::kernel::fft_filter_ccc*> d_correlators;
      filter::single_pole_iir<float,float,float>* d_avg_filter;
      std::vector<std::vector<gr_complex> > d_fir_sequences;
      std::vector<float*> d_corr_abs;

     public:
      cazac_time_sync_cc_impl(std::vector<std::vector<gr_complex> > fir_sequences, int frame_len, float threshold);
      ~cazac_time_sync_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CAZAC_TIME_SYNC_CC_IMPL_H */

