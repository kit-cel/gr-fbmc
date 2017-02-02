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

#ifndef INCLUDED_FBMC_SLIDING_FFT_CVC_IMPL_H
#define INCLUDED_FBMC_SLIDING_FFT_CVC_IMPL_H

#include <fbmc/sliding_fft_cvc.h>
#include <gnuradio/fft/fft.h>

namespace gr {
    namespace fbmc {

        class sliding_fft_cvc_impl : public sliding_fft_cvc {
        private:
            int d_subcarriers, d_overlap, d_bands, d_frame_len;
            gr::fft::fft_complex *d_fft;
          void fftshift(gr_complex* in);

        public:
            sliding_fft_cvc_impl(int subcarriers, int overlap, int bands, int frame_len);

            ~sliding_fft_cvc_impl();

            // Where all the action really happens
            void forecast(int noutput_items, gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items);
        };

    } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SLIDING_FFT_CVC_IMPL_H */

