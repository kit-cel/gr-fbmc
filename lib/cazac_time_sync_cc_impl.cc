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
    cazac_time_sync_cc::make(std::vector<std::vector<gr_complex> > fir_sequences, int frame_len, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new cazac_time_sync_cc_impl(fir_sequences, frame_len, threshold));
    }

    /*
     * The private constructor
     */
    cazac_time_sync_cc_impl::cazac_time_sync_cc_impl(std::vector<std::vector<gr_complex> > fir_sequences, int frame_len, float threshold)
      : gr::block("cazac_time_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make3(3, 3, sizeof(gr_complex), sizeof(float), sizeof(float))),
        d_threshold(threshold), d_frame_len(frame_len), d_fir_sequences(fir_sequences)
    {
      // instantiate correlators
      d_corr_abs.resize(d_fir_sequences.size());
      d_correlators.reserve(d_fir_sequences.size());
      for (int i = 0; i < d_fir_sequences.size(); ++i) {
        d_correlators.push_back(new gr::filter::kernel::fft_filter_ccc(1, d_fir_sequences[i]));
      }
      // moving average filter for power
      d_avg_filter = new filter::single_pole_iir<float,float,float>(0.5);

      set_history(d_frame_len);
      set_output_multiple(d_frame_len); // we need one frame space at the output
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
      float *corr = (float *) output_items[1];
      float *pwr = (float *) output_items[2];

      gr_complex temp[ninput_items[0]];

      float addbuf[ninput_items[0]];
      float magbuf[ninput_items[0]];
      float power[ninput_items[0]];
      memset(addbuf, 0, sizeof(float) * ninput_items[0]);

      // correlate against CAZAC preamble with FFT filters
      float temp2[ninput_items[0]];
      for (int i = 0; i < d_correlators.size(); ++i) {
        d_correlators[i]->filter(ninput_items[0], in, temp);
        volk_32fc_magnitude_32f(temp2, temp, ninput_items[0]);
        volk_32f_x2_add_32f(addbuf, addbuf, temp2, ninput_items[0]);
      }
      // TODO debug
      memcpy(corr, addbuf, sizeof(float) * ninput_items[0]);
      // power calculation
      volk_32fc_magnitude_squared_32f(magbuf, in, ninput_items[0]);
      d_avg_filter->filterN(pwr, magbuf, ninput_items[0]);

      // TODO debug
      consume_each(ninput_items[0]);
      return ninput_items[0];


      // check if threshold is met
      if(!std::any_of(addbuf, addbuf+noutput_items, [&](float f){return f >= d_threshold;} )) {
        // nothing to do this work()
        consume_each(noutput_items);
        return(0);
      }
      int peak_pos = std::distance(addbuf, std::max_element(addbuf, addbuf + noutput_items));
      memcpy(out, in+peak_pos-1120, d_frame_len);



      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */

