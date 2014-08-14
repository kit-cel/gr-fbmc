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

#ifndef INCLUDED_FBMC_frame_sync_cc_IMPL_H
#define INCLUDED_FBMC_frame_sync_cc_IMPL_H

#include <fbmc/frame_sync_cc.h>
#include <boost/circular_buffer.hpp>
#include <cstdio>
 
namespace gr {
  namespace fbmc {

    class frame_sync_cc_impl : public frame_sync_cc {
     private:
      int d_L; // number of subcarriers
      int d_frame_len; // number of symbols per frame (including preamble)
      std::vector<gr_complex> d_preamble_sym; // reference preamble symbol
      int d_step_size; // number of samples to proceed with every step
      float d_threshold; // threshold for the correlation

      bool d_tracking; // flag indicating whether the sync is in tracking (true) or acquisition mode (false)
      bool d_sync_valid; // flag indicating whether a the current sync is still vlid
      int d_num_consec_frames; // number of consecutive frames detected
      int d_sym_ctr; // number of symbols of the current frame that already have been written

      float d_f_off; // estimated frequency offset
      float d_phi_off; // estimated phase offset, updated in every step to avoid phase discontinuities

      gr_complex corr_coef(gr_complex *x1, gr_complex *x2, gr_complex *a1); // calculate a weighted correlation coefficient
      float estimate_f_off(gr_complex corr_val); // estimate frequency offset
      float estimate_phi_off(gr_complex* rx_pil); // estimate phase offset
      void correct_offsets(gr_complex* buf, float f_off, float phi_prev); // correct phase and frequency offset, start with defined phase offset to avoid discontiuities between symbols

      FILE* dbg_fp;
      FILE* dbg_fp2;

     public:
      frame_sync_cc_impl(int frame_len, std::vector<gr_complex> preamble_sym, int step_size, float threshold);
      ~frame_sync_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_frame_sync_cc_IMPL_H */

