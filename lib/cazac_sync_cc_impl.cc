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
#include "cazac_sync_cc_impl.h"
#include <numeric>

namespace gr {
  namespace fbmc {

    cazac_sync_cc::sptr
    cazac_sync_cc::make(int subcarriers, int bands, int overlap, int frame_len, float threshold,
                        std::vector<std::vector<gr_complex> > zc_seqs, std::vector<gr_complex> zc_fft) {
      return gnuradio::get_initial_sptr
          (new cazac_sync_cc_impl(subcarriers, bands, overlap, frame_len, threshold, zc_seqs, zc_fft));
    }

    /*
     * The private constructor
     */
    cazac_sync_cc_impl::cazac_sync_cc_impl(int subcarriers, int bands, int overlap, int frame_len, float threshold,
                                           std::vector<std::vector<gr_complex> > zc_seqs,
                                           std::vector<gr_complex> zc_fft)
        : gr::block("cazac_sync_cc",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
          d_frame_len(frame_len), d_threshold(threshold), d_zc_seqs(zc_seqs), d_subcarriers(subcarriers),
          d_o(overlap), d_bands(bands), d_zc_fft(zc_fft) {
      set_output_multiple(d_frame_len * d_bands * d_o * d_subcarriers);
      d_fft_len = 8192;
      d_fft = new fft::fft_complex(d_fft_len, true);
      d_curr_sample = 0;
      zero_pad(&d_zc_fft, d_fft_len); // zero pad zc fft
      d_phase = lv_cmake(1.f, 0.0f);
      d_synced = false;
    }

    /*
     * Our virtual destructor.
     */
    cazac_sync_cc_impl::~cazac_sync_cc_impl() {
      delete d_fft;
    }

    void
    cazac_sync_cc_impl::zero_pad(std::vector<gr_complex> *vector, int len) {
      int vecsize = vector->size();
      for (int i = 0; i < len - vecsize; i++) {
        vector->push_back(gr_complex(0, 0));
      }
    }

    void
    cazac_sync_cc_impl::circshift(std::vector<gr_complex> *vector, int shift) {
      vector->insert(vector->begin(), vector->end() - shift, vector->end());
      vector->erase(vector->end() - shift, vector->end());
    }

    int
    cazac_sync_cc_impl::time_sync(const gr_complex *in) {
      std::vector<gr_complex> temp(d_zc_seqs[0].size());
      std::vector<float> result(d_subcarriers * d_bands * d_o * d_frame_len - temp.size());
      // loop over each subband
      for (int j = 0; j < d_bands; j++) {
        // cross correlation
        for (int i = 0; i < d_subcarriers * d_bands * d_o * d_frame_len - temp.size(); i++) {
          volk_32fc_x2_multiply_conjugate_32fc(&temp[0], in, &d_zc_seqs[j][i], d_zc_seqs[j].size());
          result[i] += std::abs(std::accumulate(temp.begin(), temp.end(), gr_complex(0, 0)));
        }
      }
      if(*std::max_element(result.begin(), result.end()) > d_threshold) {
        d_synced = true;
        return std::distance(result.begin(), std::max_element(result.begin(), result.end()));
      }
      return 0;
    }

    void
    cazac_sync_cc_impl::freq_sync(const gr_complex *out) {
      std::vector<gr_complex> zc_received(out, out + d_subcarriers / 2);
      zero_pad(&zc_received, d_fft_len);
      memcpy(d_fft->get_inbuf(), &zc_received[0], sizeof(gr_complex) * d_fft_len);
      d_fft->execute();
      std::vector<gr_complex> temp(d_fft_len);
      std::vector<float> result(d_fft_len);
      for (int i = 0; i < d_fft_len; i++) {
        volk_32fc_x2_multiply_conjugate_32fc(&temp[0], out, &d_zc_fft[0], d_fft_len);
        result[i] += std::abs(std::accumulate(temp.begin(), temp.end(), gr_complex(0, 0)));
        circshift(&d_zc_fft, 1);
      }
      float epsilon = std::distance(result.begin(), std::max_element(result.begin(), result.end())) - d_fft_len / 2;
      epsilon /= d_fft_len;
      d_phase_increment = lv_cmake(std::cos(-2 * static_cast<float>(M_PI) * epsilon), std::sin(-2 * static_cast<float>(M_PI) * epsilon));;
    }

    void
    cazac_sync_cc_impl::freq_correction(gr_complex *out, int length) {
      volk_32fc_s32fc_x2_rotator_32fc(out, out, d_phase_increment, &d_phase, length);
    }

    void
    cazac_sync_cc_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_subcarriers * d_bands * d_o * d_frame_len;
    }

    int
    cazac_sync_cc_impl::general_work(int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      int tau = 0; // time offset
      int output_len = std::min(d_subcarriers * d_bands * d_o * d_frame_len - d_curr_sample, noutput_items);
      // Do <+signal processing+>
      if (!d_synced) { // state: not synced
        tau = time_sync(in) - (d_o + 1) * d_subcarriers + 1;
        freq_sync(&in[tau]);
      }
      if(d_synced) {
        memcpy(out, &in[tau], sizeof(gr_complex) * output_len);
        freq_correction(out, output_len);
        d_curr_sample += output_len;
        if (d_curr_sample >= d_subcarriers * d_bands * d_o * d_frame_len) {
          d_curr_sample = 0; //we're looking for a new frame now
        }
        consume_each(output_len + tau);
        return output_len;
      }

      consume_each(ninput_items[0]);
      return 0;
    }

  } /* namespace fbmc */
} /* namespace gr */

