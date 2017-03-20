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
#include "cazac_time_sync_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    cazac_time_sync_cc::sptr
    cazac_time_sync_cc::make(std::vector<std::vector<gr_complex> > fir_sequences, int frame_len, float threshold, int bands, float peak_offset)
    {
      return gnuradio::get_initial_sptr
        (new cazac_time_sync_cc_impl(fir_sequences, frame_len, threshold, bands, peak_offset));
    }

    /*
     * The private constructor
     */
    cazac_time_sync_cc_impl::cazac_time_sync_cc_impl(std::vector<std::vector<gr_complex> > fir_sequences, int frame_len, float threshold, int bands, float peak_offset)
      : gr::block("cazac_time_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_threshold(threshold), d_frame_len(frame_len), d_fir_sequences(fir_sequences), d_bands(bands)
    {
      // instantiate correlators
      d_corr_abs.resize(d_fir_sequences.size());
      d_correlators.reserve(d_fir_sequences.size());
      for (int i = 0; i < d_fir_sequences.size(); ++i) {
        // normalize taps
        std::for_each(d_fir_sequences[i].begin(), d_fir_sequences[i].end(), [&](gr_complex& c) {
          c = c/gr_complex(d_fir_sequences[i].size(), 0);
        });
        d_correlators.push_back(new gr::filter::kernel::fft_filter_ccc(1, d_fir_sequences[i]));
      }
      d_nsamps = d_correlators[0]->set_taps(d_fir_sequences[0]); // get FFT filter sample number

      // moving average filter for power
      d_avg_filter = new filter::single_pole_iir<float,float,float>(0.001);

      d_peak_offset = static_cast<int>(peak_offset * d_bands);
      d_items_left = 0;

      set_history(d_peak_offset);
      set_output_multiple(d_frame_len+d_peak_offset); // we need one frame space at the output
    }



    /*
     * Our virtual destructor.
     */
    cazac_time_sync_cc_impl::~cazac_time_sync_cc_impl()
    {
      for (int i = 0; i < d_correlators.size(); ++i) {
        delete d_correlators[i];
      }
      delete d_avg_filter;
    }

    void
    cazac_time_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items; // pretend to be a sync block since there is no mathematical expression
                                                // that describes our behaviour
    }

    int
    cazac_time_sync_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // we have not finished the last frame yet
      if(d_items_left > 0) {
        int emit = std::min(d_items_left, ninput_items[0]);
        memcpy(out, in+d_peak_offset, emit * sizeof(gr_complex));
        d_items_left -= emit;
        consume_each(emit);
        return emit;
      }
      // fft filter kernel wants specific number of samples in each call
      int num_items = d_nsamps*(ninput_items[0]/d_nsamps); // round down to nearest multiple of d_nsamps

      // allocate temporary buffers
      gr_complex* temp = (gr_complex*) volk_malloc(sizeof(gr_complex) * num_items, volk_get_alignment());
      float* temp2 = (float*) volk_malloc(sizeof(float) * num_items, volk_get_alignment());
      float* addbuf = (float*) volk_malloc(sizeof(float) * num_items, volk_get_alignment());
      float* magbuf = (float*) volk_malloc(sizeof(float) * num_items, volk_get_alignment());
      float* power = (float*) volk_malloc(sizeof(float) * num_items, volk_get_alignment());
      memset(addbuf, 0, sizeof(float) * num_items);

      // correlate against CAZAC preamble with FFT filters
      for (int i = 0; i < d_correlators.size(); ++i) {
        d_correlators[i]->filter(num_items, in, temp);
        volk_32fc_magnitude_32f(temp2, temp, num_items);
        volk_32f_x2_add_32f(addbuf, addbuf, temp2, num_items);
      }

      // power estimation
      volk_32fc_magnitude_32f(magbuf, in, num_items); // according to correlation coefficient we don't need to square
                                                      // here because of normalized taps sqrt(e^2 * 1^2) = e
      d_avg_filter->filterN(power, magbuf, num_items);

      // normalize correlation
      volk_32f_x2_divide_32f(addbuf, addbuf, power, num_items);

      // check if threshold is met
      if(!std::any_of(addbuf, addbuf+noutput_items, [&](float f){return f >= d_threshold/(float)d_bands;} )) {
        // nothing to do this work()
        volk_free(temp);
        volk_free(temp2);
        volk_free(addbuf);
        volk_free(magbuf);
        volk_free(power);
        consume_each(noutput_items);
        return(0);
      }
      int frame_start = std::distance(addbuf, std::max_element(addbuf, addbuf + num_items))-d_peak_offset; // argmax
      d_items_left = d_frame_len;
      int emit = std::min(d_frame_len, ninput_items[0]-frame_start);
      memcpy(out, in+frame_start, emit * sizeof(gr_complex));
      d_items_left -= emit;
      volk_free(temp);
      volk_free(temp2);
      volk_free(addbuf);
      volk_free(magbuf);
      volk_free(power);
      consume_each (emit);

      // Tell runtime system how many output items we produced.
      return emit;
    }

  } /* namespace fbmc */
} /* namespace gr */

