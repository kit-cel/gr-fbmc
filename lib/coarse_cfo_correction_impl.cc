/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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
#include "coarse_cfo_correction_impl.h"
#include <numeric>
#include "volk/volk.h"

namespace gr {
  namespace fbmc {

    coarse_cfo_correction::sptr
    coarse_cfo_correction::make(std::vector<int> channel_map)
    {
      return gnuradio::get_initial_sptr
        (new coarse_cfo_correction_impl(channel_map));
    }

    /*
     * The private constructor
     */
    coarse_cfo_correction_impl::coarse_cfo_correction_impl(std::vector<int> channel_map)
      : gr::block("coarse_cfo_correction",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      d_L = channel_map.size();
      d_channel_map.assign(&channel_map[0], &channel_map[0]+channel_map.size());

      // set up FFTW parameters
      d_interp_fac = 8; // 2 is the theoretical minimum
      d_nfft = d_interp_fac*d_L;
      d_buf = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*d_nfft);
      d_buf_abs.assign(d_nfft, 0);
      d_plan = fftwf_plan_dft_1d(d_nfft, d_buf, d_buf, FFTW_FORWARD, FFTW_ESTIMATE);

      // prepare the filter windows (frequency domain)
      d_delta = 2; // increasing this value makes the estimation better but reduces the lock range
      generate_filter_windows();

      set_output_multiple(d_nfft);
    }

    /*
     * Our virtual destructor.
     */
    coarse_cfo_correction_impl::~coarse_cfo_correction_impl()
    {
      fftwf_free(d_buf);
      fftwf_destroy_plan(d_plan);
    }

    void
    coarse_cfo_correction_impl::generate_filter_windows()
    {
      int upper_edge = 0; // highest used carrier
      int lower_edge = 0; // lowest used carrier
      for(int i=1; i<d_L; i++)
      {
        if(d_channel_map[i] == 0)
        {
          upper_edge = i-1;
          break;
        }
      }
      for(int i=d_L/2; i<d_L; i++)
      {
        if(d_channel_map[i] != 0)
        {
          lower_edge = i;
          break;
        }
      }
      if(upper_edge == 0 || lower_edge == 0 || std::accumulate(d_channel_map.begin(), d_channel_map.end(), 0) != upper_edge + (d_L - lower_edge))
      {
        //std::cout << "upper: " << upper_edge << ", lower: " << lower_edge << ", accumulate: " << std::accumulate(d_channel_map.begin(), d_channel_map.end(), 0) << " != " << upper_edge + (d_L - lower_edge) << std::endl;
        throw std::runtime_error("Channel map is assumed to be DC free and to have no empty carriers in the side bands");
      }

      // generate the prototypes of the frequency domain filters
      boost::circular_buffer<int> proto_upper_filter(d_nfft, 0);
      boost::circular_buffer<int> proto_lower_filter(d_nfft, 0);
      for(int i=-d_delta*d_interp_fac; i<d_delta*d_interp_fac; i++)
      {
        proto_upper_filter[upper_edge*d_interp_fac + i] = 1;
        proto_lower_filter[lower_edge*d_interp_fac + i] = 1;
      }

      // calculate the maximum shift indices so that the pass bands dont wrap around the edges of the spectrum
      int n_shifts_up = (d_L-1-upper_edge)*d_interp_fac;
      int n_shifts_down = (lower_edge-d_L/2)*d_interp_fac;
      d_shifts.clear();
      for(int i=-n_shifts_down; i<=n_shifts_up; i++)
        d_shifts.push_back(i);

      // create all the differently shifted versions of the band pass filters
      boost::circular_buffer<int> tmp_lower_filter = proto_lower_filter;
      boost::circular_buffer<int> tmp_upper_filter = proto_upper_filter;

      // set up the lowest version of the filters
      tmp_upper_filter.rotate(tmp_upper_filter.begin()+n_shifts_down);
      tmp_lower_filter.rotate(tmp_lower_filter.begin()+n_shifts_down);

      // shift them up one by one and save a copy in the corresponding matrix
      std::vector<float> tmp_upper_v;
      std::vector<float> tmp_lower_v;
      for(int i=0; i<d_shifts.size(); i++)
      {
        tmp_upper_v.assign(tmp_upper_filter.begin(), tmp_upper_filter.end());
        d_w_u.push_back(tmp_upper_v);

        tmp_lower_v.assign(tmp_lower_filter.begin(), tmp_lower_filter.end());
        d_w_l.push_back(tmp_lower_v);

        // rotate the vector so that the last item is now the first item and therefore the vector is shifted by 1
        tmp_upper_filter.rotate(tmp_upper_filter.end()-1);
        tmp_lower_filter.rotate(tmp_lower_filter.end()-1);
      }
    }

    int
    coarse_cfo_correction_impl::find_optimal_shift()
    {
      // calculate energy difference between the two bandpass filters
      d_energy_diff.assign(d_shifts.size(), -1);
      float e_upper, e_lower;
      for(int i=0; i<d_shifts.size(); i++)
      {
        volk_32f_x2_dot_prod_32f(&e_upper, &d_buf_abs[0], &d_w_u[i][0], d_nfft);
        volk_32f_x2_dot_prod_32f(&e_lower, &d_buf_abs[0], &d_w_l[i][0], d_nfft);
        d_energy_diff[i] = abs(e_upper-e_lower);
      }

      // find the minimum
      float e_min = d_energy_diff[0];
      int i_min = 0;
      for(int i=1; i<d_energy_diff.size(); i++)
      {
        if(d_energy_diff[i] < e_min)
        {
          e_min = d_energy_diff[i];
          i_min = i;
        }
      }

      return d_shifts[i_min];
    }

    bool
    coarse_cfo_correction_impl::check_signal_presence()
    {
      std::cout << "signal presence check not implemented" << std::endl;
      // use d_buf_abs and the calculated shift to compare the energy in the band vs the out-of-band noise
      return false;
    }

    void
    coarse_cfo_correction_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = d_nfft;
    }

    int
    coarse_cfo_correction_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        consume_each (d_nfft);

        if(!d_signal_found)
        {
          // copy input samples to FFT buffer, execute FFT, shift the output
          memcpy(d_buf, in, sizeof(gr_complex)*d_nfft);
          fftwf_execute(d_plan);
          for(int i=0; i<d_nfft; i++)
            d_buf_abs[i] = abs(gr_complex(d_buf[i][0], d_buf[i][1]));

          int opt_shift = find_optimal_shift();
          d_signal_found = check_signal_presence();
          d_cfo = 1.0*opt_shift/d_nfft;
          std::cout << "calculated coarse CFO: " << d_cfo*250e3 << " Hz. Shift: " << opt_shift << std::endl;
        }

        for(int i=0; i<d_nfft; i++)
          out[i] = in[i]*exp(gr_complex(0,-1*2*M_PI*d_cfo*i));

        // Tell runtime system how many output items we produced.
        return d_nfft;
    }

  } /* namespace fbmc */
} /* namespace gr */

