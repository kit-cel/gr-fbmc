/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_FBMC_TIME_FREQ_SYNC_CC_IMPL_H
#define INCLUDED_FBMC_TIME_FREQ_SYNC_CC_IMPL_H

#include <fbmc/time_freq_sync_cc.h>

namespace gr {
  namespace fbmc {

    class time_freq_sync_cc_impl : public time_freq_sync_cc {
    private:
      int d_L;
      float d_threshold;
      int d_nsym_frame;
      int d_nsamp_frame;
      int d_lookahead;
      gr_complex d_corrbuf;
      uint8_t d_state;
      float d_phi;
      float d_cfo;
      int d_nsamp_remaining;
      int d_stepsize;
      int d_additional_samps;

      void enter_track_state();
      void corr_remove_old(const gr_complex* buf, int pos);
      void corr_add_new(const gr_complex* buf, int pos);

    public:
      time_freq_sync_cc_impl(int L, float threshold, int nsym_frame, int stepsize, int additional_samps);

      ~time_freq_sync_cc_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_TIME_FREQ_SYNC_CC_IMPL_H */

