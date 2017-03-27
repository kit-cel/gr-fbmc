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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "cazac_freq_sync_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    cazac_freq_sync_cc::sptr
    cazac_freq_sync_cc::make(int subcarriers, int bands, int frame_len, int fft_size, std::vector<gr_complex> fft_sequences)
    {
      return gnuradio::get_initial_sptr
        (new cazac_freq_sync_cc_impl(subcarriers, bands, frame_len, fft_size, fft_sequences));
    }

    /*
     * The private constructor
     */
    cazac_freq_sync_cc_impl::cazac_freq_sync_cc_impl(int subcarriers, int bands, int frame_len, int fft_size, std::vector<gr_complex> fft_sequences)
      : gr::sync_block("cazac_freq_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_bands(bands), d_fft_sequences(fft_sequences), d_subcarriers(subcarriers), d_frame_len(frame_len)
    {
      d_curr_samp = 0;
      d_fo = 0;
      d_range = 16 * d_bands; // affects tracking range of frequency offset
      d_fft_len = fft_size * d_bands;
      d_phase = lv_cmake(1, 0); //phase for rotation
      d_fft = new gr::fft::fft_complex(d_fft_len, true);
      if(d_fft_sequences.size() != d_fft_len) {
        std::cout << "FFT Size = " << d_fft_len << std::endl;
        std::cout << "Taps Size = " << d_fft_sequences.size() << std::endl;
        throw std::runtime_error("Wrong FFT size!");
      };
      set_output_multiple(d_bands * 5.0/2.0 * d_subcarriers);
    }

    float
    cazac_freq_sync_cc_impl::get_fo() {
      return d_fo;
    }

    /*
     * Our virtual destructor.
     */
    cazac_freq_sync_cc_impl::~cazac_freq_sync_cc_impl()
    {
      delete d_fft;
    }

    float
    cazac_freq_sync_cc_impl::get_freq_offset(gr_complex* in) {
      std::vector<float> correlation;
      // calculate first half of correlation vector
      for (int r = d_range; r > 0; r--) {
        volk_32fc_x2_conjugate_dot_prod_32fc(&d_temp, in, d_fft_sequences.data() + r,
                                               d_fft_len-d_range);
        correlation.push_back(std::abs(d_temp));
      }
      // calculate second half of correlation vector
      for (int r = 0; r < d_range; r++) {
        volk_32fc_x2_conjugate_dot_prod_32fc(&d_temp, in + r, d_fft_sequences.data(),
                                             d_fft_len-d_range);
        correlation.push_back(std::abs(d_temp));
      }
      // argmax(correlation) - mean
      float fo = std::distance(correlation.begin(), std::max_element(correlation.begin(), correlation.end())) - d_range;
      return fo/d_fft_len;
    }

    int
    cazac_freq_sync_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // get either max output buffer size or frame length to produce
      int emit = std::min(noutput_items, d_frame_len-d_curr_samp);

      // frame start -> search for CAZAC and estimate frequency offset
      if(d_curr_samp == 0) {
        //copy CAZAC into fft
        memcpy(d_fft->get_inbuf(), in + 2 * d_subcarriers * d_bands, d_subcarriers/2 * d_bands * sizeof(gr_complex));
        d_fft->execute();
        //estimate frequency offset
        d_fo = 0.8*d_fo + 0.2 * get_freq_offset(d_fft->get_outbuf()); // averaging frequency offset
        //calculate phase increment for new frequency correction
        float arg = -2*M_PI * d_fo;
        d_phase_inc = lv_cmake(std::cos(arg), std::sin(arg));
      }
      // correct frequency offset
      volk_32fc_s32fc_x2_rotator_32fc(out, in, d_phase_inc, &d_phase, emit);

      d_curr_samp += emit;
      // if we have emitted a whole frame, reset current sample index
      if(d_curr_samp == d_frame_len) {
        d_curr_samp = 0;
      }

      // Tell runtime system how many output items we produced.
      return emit;
    }

  } /* namespace fbmc */
} /* namespace gr */

