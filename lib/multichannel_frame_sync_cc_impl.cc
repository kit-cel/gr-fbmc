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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include "multichannel_frame_sync_cc_impl.h"
#include <fbmc/tx_sdft_kernel.h>

namespace gr {
  namespace fbmc {

    multichannel_frame_sync_cc::sptr
    multichannel_frame_sync_cc::make(int L, int nsym_frame, std::vector<gr_complex> preamble_sym, std::vector<float> taps, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new multichannel_frame_sync_cc_impl(L, nsym_frame, preamble_sym, taps, step_size, threshold));
    }

    /*
     * The private constructor
     */
    multichannel_frame_sync_cc_impl::multichannel_frame_sync_cc_impl(int L, int nsym_frame, std::vector<gr_complex> preamble_sym, std::vector<float> taps, int step_size, float threshold)
      : gr::block("multichannel_frame_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
              d_L(L), d_nsym_frame(nsym_frame), d_step_size(step_size),
              d_threshold(threshold), d_nsamp_frame(d_L * d_nsym_frame / 2)
    {
      d_search_window_fixedlag = d_nsamp_frame;
      d_fixedlag_lookahead = 3*d_L/2;
      d_search_window_reference = d_L;
      d_search_buf = (gr_complex*) volk_malloc(sizeof(gr_complex)*(d_nsamp_frame+d_search_window_reference), volk_get_alignment());
      prepare_mixed_preambles(preamble_sym, taps);
      volk_32fc_x2_conjugate_dot_prod_32fc(&d_preamble_energy, &d_mixed_preamble[0][0], &d_mixed_preamble[0][0], d_mixed_preamble[0].size());
      set_output_multiple(d_nsamp_frame); // length of one entire frame
    }

    /*
     * Our virtual destructor.
     */
    multichannel_frame_sync_cc_impl::~multichannel_frame_sync_cc_impl()
    {
    }

    void
    multichannel_frame_sync_cc_impl::prepare_mixed_preambles(std::vector<gr_complex> preamble, std::vector<float> taps)
    {
      // initialize kernel
      for(int i=0; i<d_num_subchannels; i++)
      {
        tx_sdft_kernel kernel(taps, d_L);
        int nsym_preamble = preamble.size()/(d_L/d_num_subchannels);
        int nsym_sync = nsym_preamble + d_overlap;
        std::vector<gr_complex> symbols(d_L*nsym_sync, 0);
        d_mixed_preamble.push_back(std::vector<gr_complex>(d_L/2 * nsym_sync, 0));
        for(int k=0; k<nsym_preamble; k++)
        {
          memcpy(&symbols[k*d_L + i*d_L/d_num_subchannels], &preamble[k*d_L/d_num_subchannels], sizeof(gr_complex)*d_L/d_num_subchannels);
        }
        kernel.generic_work(&d_mixed_preamble[i][0], &symbols[0], nsym_sync*d_L/2);
      }

    }

    bool
    multichannel_frame_sync_cc_impl::fixed_lag_correlation(const gr_complex* in, gr_complex& corr_coef)
    {
      // results for auto- and crosscorrelation
      gr_complex xcorr = 0;
      gr_complex acorr = 0;

      volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, in + d_fixedlag_lookahead, in + d_fixedlag_lookahead + d_L, d_L);
      volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, in + d_fixedlag_lookahead, in + d_fixedlag_lookahead, d_L * 2);

      // acorr is calculated over two symbols, so scale accordingly
      corr_coef = gr_complex(2, 0) * xcorr / acorr;
      if(std::abs(corr_coef) > d_threshold){ return true; }
      else{ return false; }
    }

    double
    multichannel_frame_sync_cc_impl::calc_cfo(gr_complex c)
    {
      return -1.0 / (2 * M_PI * d_L) * arg(c);
    }

    void
    multichannel_frame_sync_cc_impl::correct_frequency_offset(double cfo_norm)
    {
      // possible optimization options:
      // I) prepare vector and use VOLK for multiplication
      // II) sin/cos representation
//      std::cout << "normalized CFO: " << cfo_norm << std::endl;
      for(int i=0; i<d_nsamp_frame+d_search_window_reference; i++)
      {
        d_search_buf[i] *= exp(gr_complex(0, -2 * M_PI * cfo_norm * i));
      }
    }

    bool
    multichannel_frame_sync_cc_impl::multichannel_detection(gr_complex* buf, std::vector<gr_complex>& corr_coefs,
                                std::vector<bool>& occupied_channels, int& index)
    {
      corr_coefs.clear();
      occupied_channels.clear();
      index = 0;

      std::vector< std::vector<gr_complex> > corr_vec;
      std::vector<gr_complex> corr_sum(d_search_window_reference, 0);
      gr_complex signal_energy = 0;
      gr_complex tmp = 0;
      for(int i=0; i<d_search_window_reference; i++)
      {
        corr_vec.push_back(std::vector<gr_complex>(d_num_subchannels, 0));
        volk_32fc_x2_conjugate_dot_prod_32fc(&signal_energy, buf+i, buf+i, d_mixed_preamble[0].size());
        for(int k=0; k<d_num_subchannels; k++)
        {
          volk_32fc_x2_conjugate_dot_prod_32fc(&tmp, buf+i, &d_mixed_preamble[k][0], d_mixed_preamble[k].size());
          corr_vec[i][k] = tmp / sqrt(d_preamble_energy * signal_energy);
          corr_sum[i] += corr_vec[i][k];
        }
        corr_sum[i] = std::abs(corr_sum[i]);
        if(i > 0 and corr_sum[i].real() > corr_sum[index].real() and corr_sum[i].real() > d_threshold/d_num_subchannels)
        {
          index = i;
        }
      }

      if(index == 0 and corr_sum[0].real() < d_threshold/d_num_subchannels) // make sure index has been moved or is already at peak position
      {
        return false;
      }
      else
      {
        // find maximum
        float maxval = 0;
        std::vector<float> corr_abs;
        for(int i=0; i<d_num_subchannels; i++)
        {
          corr_abs.push_back(std::abs(corr_vec[index][i]));
          if(corr_abs[i] > maxval)
          {
            maxval = corr_abs[i];
          }
        }
        for(int i=0; i<d_num_subchannels; i++)
        {
          if(corr_abs[i] > maxval/2)
          {
            occupied_channels.push_back(true);
            corr_coefs.push_back(corr_vec[index][i]);
          }
          else
          {
            occupied_channels.push_back(false);
          }
        }
        return true;
      }
    }

    void
    multichannel_frame_sync_cc_impl::correct_phase_offset(std::vector <gr_complex> corr_coefs, int bufpos)
    {
      // average the phases of the corr_coefs
      double angle = 0;
//      std::cout << "phase angles: ";
      for(int i=0; i<corr_coefs.size(); i++)
      {
//        std::cout << arg(corr_coefs[i]) << " ";
        angle+= arg(corr_coefs[i]);
      }
      angle /= corr_coefs.size();
//      std::cout << ", average: " << angle << std::endl;
      gr_complex phi = exp(gr_complex(0, -angle));
      volk_32fc_s32fc_multiply_32fc(d_search_buf + bufpos, d_search_buf + bufpos, phi, d_nsamp_frame);
    }

    void
    multichannel_frame_sync_cc_impl::add_channel_occupation_tag(const std::vector <bool> &occupied_channels)
    {
      pmt::pmt_t pmt_vec = pmt::make_vector(d_num_subchannels, pmt::PMT_F);
      for(int i=0; i<d_num_subchannels; i++)
      {
        if(occupied_channels[i])
        {
          pmt::vector_set(pmt_vec, i, pmt::PMT_T);
        }
      }
      add_item_tag(0, nitems_written(0), pmt::mp("occupied_subchannels"), pmt_vec);
    }

    void
    multichannel_frame_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = d_nsamp_frame + d_search_window_fixedlag + d_search_window_reference; // length of one entire frame plus search windows
    }

    int
    multichannel_frame_sync_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      int consumed = 0;

      // I. detect frame via fixed lag correlation
      gr_complex corr_coef;
      bool proceed = false;
      while(consumed < d_search_window_fixedlag - d_L) // make sure there is enough space for some reference correlation iterations
      {
        proceed = fixed_lag_correlation(in+consumed, corr_coef);
        if(proceed) { break; }
        else{ consumed += d_step_size; }
      }
      // proceed?
      if(!proceed)
      {
        std::cout << "frame_sync: FLC not successful" << std::endl;
        consume_each(consumed);
        return 0;
      }

      // II. correct frequency offset
//      std::cout << "FLC positive @" << nitems_read(0) + consumed << std::endl;
      memcpy(d_search_buf, in+consumed, sizeof(gr_complex)*(d_nsamp_frame+d_search_window_reference));
      double normalized_cfo = calc_cfo(corr_coef);
      correct_frequency_offset(normalized_cfo); // TODO: check this

      // III. detect used subchannels via reference correlations
      std::vector<bool> occupied_channels;
      std::vector<gr_complex> corr_coefs;
      int bufpos = 0;
      proceed = multichannel_detection(d_search_buf, corr_coefs, occupied_channels, bufpos); // returns true if at least one subchannel was detected
      consumed += bufpos;
      // proceed?
      if(!proceed)
      {
        std::cout << "no preamble found" << std::endl;
        consume_each(consumed);
        return 0;
      }

      // IV. correct remaining phase offset
      correct_phase_offset(corr_coefs, bufpos);

      // V. copy to output buffer and add tags denoting subchannel occupation
//      std::cout << "frame detected @" << nitems_read(0) + consumed << std::endl;
      memcpy(out, d_search_buf+bufpos, sizeof(gr_complex)*d_nsamp_frame);
      add_channel_occupation_tag(occupied_channels);

      // VI. consume and return entire frame
      consumed += d_nsamp_frame;

      consume_each(consumed);
      return d_nsamp_frame;
    }

  } /* namespace fbmc */
} /* namespace gr */

