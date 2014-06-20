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

#ifndef INCLUDED_FBMC_FRAME_SYNC_CVC_IMPL_H
#define INCLUDED_FBMC_FRAME_SYNC_CVC_IMPL_H

#include <fbmc/frame_sync_cvc.h>
#include <boost/circular_buffer.hpp>

namespace gr {
  namespace fbmc {

    class frame_sync_cvc_impl : public frame_sync_cvc
    {
     private:
      int d_L; // number of subcarriers
      int d_frame_len; // number of symbols per frame (including preamble)
      int d_overlap; // number of overlapping symbols
      std::string d_preamble; // preamble type
      int d_step_size; // number of samples to proceed with every step
      float d_threshold; // threshold for the correlation

      int d_num_hist_sym; // number of symbols that have to be kept in history to be able to return a whole frame when the start was found
      std::vector<gr_complex> d_history; // keeps a history of past symbols
      bool d_frame_found; // flag indicating wheter a frame start has been found
      int d_sym_ctr; // number of symbols of the current frame that already have been written

      float corr_coef(gr_complex *x1, gr_complex *x2, gr_complex *a1); // calculate a weighted correlation coefficient

     public:
      frame_sync_cvc_impl(int L, int frame_len, int overlap, std::string preamble, int step_size, float threshold);
      ~frame_sync_cvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_SYNC_CVC_IMPL_H */

