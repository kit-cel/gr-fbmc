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
//      interpolate_preamble(preamble_sym);
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

//    void
//    multichannel_frame_sync_cc_impl::interpolate_preamble(std::vector<gr_complex> preamble)
//    {
//      // preparations
//      int ntaps = 121;
//      float taps[121] = {-0.00024929261417128146, 2.7295618565403856e-05, 0.00030980337760411203, 0.00047427925164811313, 0.0004318858846090734, 0.00017098820535466075, -0.00022229834576137364, -0.0005814371397718787, -0.0007196675287559628, -0.0005173816462047398, 4.414816371199137e-18, 0.0006381183047778904, 0.0010926378890872002, 0.0010824397904798388, 0.0005044178105890751, -0.00046904696500860155, -0.0014173350064083934, -0.0018385483417659998, -0.0013978359056636691, -0.0001410132390446961, 0.0014490133617073298, 0.0026161817368119955, 0.0026729267556220293, 0.0013732216320931911, -0.0008742373320274055, -0.003095031948760152, -0.0041490704752504826, -0.003288743318989873, -0.0006128582172095776, 0.0028366234619170427, 0.005445413291454315, 0.005752664525061846, 0.003218530910089612, -0.001350944396108389, -0.005991968791931868, -0.008385613560676575, -0.00696625467389822, -0.0018266135593876243, 0.005064631346613169, 0.010546242818236351, 0.011647133156657219, 0.007091585546731949, -0.00179487862624228, -0.011314144358038902, -0.016821611672639847, -0.014844637364149094, -0.004982819315046072, 0.00935021135956049, 0.021877693012356758, 0.02593032643198967, 0.017544684931635857, -0.0021079706493765116, -0.026136113330721855, -0.043705884367227554, -0.043754469603300095, -0.019350994378328323, 0.02898005023598671, 0.09238900989294052, 0.15583491325378418, 0.20263129472732544, 0.21984902024269104, 0.20263129472732544, 0.15583491325378418, 0.09238900989294052, 0.02898005023598671, -0.019350994378328323, -0.043754469603300095, -0.043705884367227554, -0.026136113330721855, -0.0021079706493765116, 0.017544684931635857, 0.02593032643198967, 0.021877693012356758, 0.00935021135956049, -0.004982819315046072, -0.014844637364149094, -0.016821611672639847, -0.011314144358038902, -0.00179487862624228, 0.007091585546731949, 0.011647133156657219, 0.010546242818236351, 0.005064631346613169, -0.0018266135593876243, -0.00696625467389822, -0.008385613560676575, -0.005991968791931868, -0.001350944396108389, 0.003218530910089612, 0.005752664525061846, 0.005445413291454315, 0.0028366234619170427, -0.0006128582172095776, -0.003288743318989873, -0.0041490704752504826, -0.003095031948760152, -0.0008742373320274055, 0.0013732216320931911, 0.0026729267556220293, 0.0026161817368119955, 0.0014490133617073298, -0.0001410132390446961, -0.0013978359056636691, -0.0018385483417659998, -0.0014173350064083934, -0.00046904696500860155, 0.0005044178105890751, 0.0010824397904798388, 0.0010926378890872002, 0.0006381183047778904, 4.414816371199137e-18, -0.0005173816462047398, -0.0007196675287559628, -0.0005814371397718787, -0.00022229834576137364, 0.00017098820535466075, 0.0004318858846090734, 0.00047427925164811313, 0.00030980337760411203, 2.7295618565403856e-05, -0.00024929261417128146};
//
//      int group_delay = (ntaps-1)/2;
//      std::vector<gr_complex> tmp(ntaps+preamble.size()*d_num_subchannels, 0);
//      d_interp_preamble_sym = std::vector<gr_complex>(d_num_subchannels*preamble.size(), 0);
//
//      // upsampling
//      for(int i=0; i<preamble.size(); i++)
//      {
//        tmp[group_delay + i*d_num_subchannels] = preamble[i];
//      }
//
//      // filtering (image rejection)
//      for(int i=0; i<d_interp_preamble_sym.size(); i++)
//      {
//        volk_32fc_32f_dot_prod_32fc(&d_interp_preamble_sym[i], &tmp[i], taps, ntaps);
//      }
//    }

    void
    multichannel_frame_sync_cc_impl::prepare_mixed_preambles(std::vector<gr_complex> preamble, std::vector<float> taps)
    {
      // initialize kernel
      for(int i=0; i<d_num_subchannels; i++)
      {
        tx_sdft_kernel kernel(taps, d_L);
        int nsym_preamble = preamble.size()/(d_L/d_num_subchannels);
        int nsym_sync = nsym_preamble + d_overlap;
        std::cout << "nsym_preamble: " << nsym_preamble << std::endl;
        std::vector<gr_complex> symbols(d_L*nsym_sync, 0);
        d_mixed_preamble.push_back(std::vector<gr_complex>(d_L/2 * nsym_sync, 0));
        std::cout << "len(symbols): " << symbols.size() << ", len(mixed_preamble): " << d_mixed_preamble[i].size() << std::endl;
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
      return -1.0 / (2 * M_PI * d_L) * arg(c); // TODO: check this
    }

    void
    multichannel_frame_sync_cc_impl::correct_frequency_offset(double cfo_norm)
    {
      // possible optimization options:
      // I) prepare vector and use VOLK for multiplication
      // II) sin/cos representation
      for(int i=0; i<d_nsamp_frame+d_search_window_reference; i++)
      {
        d_search_buf[i] *= exp(gr_complex(0, -2 * M_PI * cfo_norm * i));
      }
    }

    bool
    multichannel_frame_sync_cc_impl::multichannel_detection(gr_complex* buf, std::vector<gr_complex>& corr_coefs,
                                std::vector<bool>& occupied_channels)
    {
      bool channel_detected = false;
      corr_coefs.clear();
      occupied_channels.clear();

      gr_complex signal_energy = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&signal_energy, buf, buf, d_mixed_preamble[0].size());

      gr_complex dotprod = 0;
//      std::cout << "frame_sync: reference correlation coefficients: \n";
      for(int i=0; i<d_num_subchannels; i++)
      {
        volk_32fc_x2_conjugate_dot_prod_32fc(&dotprod, buf, &d_mixed_preamble[i][0], d_mixed_preamble[i].size());
        corr_coefs.push_back(dotprod / sqrt(d_preamble_energy * signal_energy));
//        std::cout << "\t#" << i << ": " << abs(corr_coefs[i]) << " = " << abs(dotprod) << "/ sqrt(" << abs(d_preamble_energy) << "*" << abs(signal_energy) << ")" << std::endl;
        if(abs(corr_coefs[i]) > d_threshold)
        {
          channel_detected = true;
          occupied_channels.push_back(true);
        }
        else
        {
          occupied_channels.push_back(false);
        }
      }
      return channel_detected;
    }

    void
    multichannel_frame_sync_cc_impl::correct_phase_offset(std::vector <gr_complex> corr_coefs)
    {
      // average the phases of the corr_coefs
      double angle = 0;
      for(int i=0; i<corr_coefs.size(); i++)
      {
        angle+= arg(corr_coefs[i]);
      }
      angle /= corr_coefs.size();
      gr_complex phi = exp(gr_complex(0, -2 * M_PI * angle)); // TODO: check this
      volk_32fc_s32fc_multiply_32fc(d_search_buf, d_search_buf, phi, d_nsamp_frame + d_search_window_reference);
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
      //correct_frequency_offset(normalized_cfo); // TODO: check this

      // III. detect used subchannels via reference correlations
      std::vector<bool> occupied_channels;
      std::vector<gr_complex> corr_coefs;
      int pos = 0;
      while(pos < d_search_window_reference)
      {
//        std::cout << pos << "/" << d_search_window_reference << " RC @" << nitems_read(0)+consumed << std::endl;
        proceed = multichannel_detection(d_search_buf+pos, corr_coefs, occupied_channels); // returns true if at least one subchannel was detected
        if(proceed) { /*std::cout << "preamble found!" << std::endl;*/ break; }
        else{ consumed += 1; }
        pos++;
      }
//      std::cout << "frame_sync: preamble detected: " << proceed << std::endl;
      // proceed?
      if(!proceed)
      {
        std::cout << "no preamble found" << std::endl;
        consume_each(consumed);
        return 0;
      }

      // IV. correct remaining phase offset
//      correct_phase_offset(corr_coefs); // TODO: this might not work properly

      // V. copy to output buffer and add tags denoting subchannel occupation
      memcpy(out, d_search_buf+pos, sizeof(gr_complex)*d_nsamp_frame);
      add_channel_occupation_tag(occupied_channels);

      // VI. consume and return entire frame
      consumed += d_nsamp_frame;

      consume_each(consumed);
      return d_nsamp_frame;
    }

  } /* namespace fbmc */
} /* namespace gr */

