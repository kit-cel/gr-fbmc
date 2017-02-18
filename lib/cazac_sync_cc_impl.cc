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
#include <volk/volk.h>
#include <numeric>

namespace gr {
  namespace fbmc {

    cazac_sync_cc::sptr
    cazac_sync_cc::make(int subcarriers, int bands, int overlap, int frame_len, float threshold, std::vector<std::vector<gr_complex> >& zc_seqs)
    {
      return gnuradio::get_initial_sptr
        (new cazac_sync_cc_impl(subcarriers, bands, overlap, frame_len, threshold, zc_seqs));
    }

    /*
     * The private constructor
     */
    cazac_sync_cc_impl::cazac_sync_cc_impl(int subcarriers, int bands, int overlap, int frame_len, float threshold, std::vector<std::vector<gr_complex> >& zc_seqs)
      : gr::block("cazac_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_frame_len(frame_len), d_threshold(threshold), d_zc_seqs(zc_seqs), d_subcarriers(subcarriers),
        d_o(overlap), d_bands(bands)
    {
      set_output_multiple(d_frame_len * d_o * d_subcarriers);
      d_fft = new fft::fft_complex(8192, true);
    }

    /*
     * Our virtual destructor.
     */
    cazac_sync_cc_impl::~cazac_sync_cc_impl()
    {
      delete d_fft;
    }

    int
    cazac_sync_cc_impl::time_sync(const gr_complex* in) {
      std::vector<gr_complex> temp(d_zc_seqs[0].size());
      std::vector<float> result(d_subcarriers * d_o * d_frame_len - temp.size());
      // loop over each subband
      for (int j = 0; j < d_bands; j++) {
        // cross correlation
        for (int i = 0; i < d_subcarriers * d_o * d_frame_len - temp.size(); i++) {
          volk_32fc_x2_multiply_conjugate_32fc(&temp[0], in, &d_zc_seqs[j][i], d_zc_seqs[j].size());
          result[i] += std::abs(std::accumulate(temp.begin(), temp.end(), gr_complex(0, 0)));
        }
      }
      return std::distance(result.begin(), std::max_element(result.begin(), result.end()));
    }

    void
    cazac_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = d_subcarriers * d_o * d_frame_len;
    }

    int
    cazac_sync_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      int tau = time_sync(in) - (d_o+1) * d_subcarriers + 1;
      in += tau; // time sync
      // TODO freq sync

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

