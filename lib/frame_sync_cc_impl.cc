/* -*- c++ -*- */
/* 
 * Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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
#include "frame_sync_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    frame_sync_cc::sptr
    frame_sync_cc::make(int L, int frame_len, int overlap, std::string preamble, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new frame_sync_cc_impl(L, frame_len, overlap, preamble, step_size, threshold));
    }

    /*
     * The private constructor
     */
    frame_sync_cc_impl::frame_sync_cc_impl(int L, int frame_len, int overlap, std::string preamble, int step_size, float threshold)
      : gr::block("frame_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
                d_L(L),
                d_frame_len(frame_len),
                d_overlap(overlap),
                d_preamble(preamble),
                d_step_size(step_size),
                d_threshold(threshold),
                d_num_hist_samp(1),
                d_sync_valid(false),
                d_tracking(false),
                d_num_consec_frames(0),
                d_sym_ctr(0)
    {
      if(d_step_size != 1)
        throw std::runtime_error(std::string("Step size must be 1 at the moment because there is no sensible use implemented yet"));

      if(d_threshold <= 0 || d_threshold >= 1)
        throw std::runtime_error(std::string("Threshold must be between (0,1)")); 

      if(d_step_size > d_L)
        throw std::runtime_error(std::string("Step size must be smaller than or equal to the symbol length"));

      if(d_preamble != "IAM")
        throw std::runtime_error(std::string("Only IAM is supported"));

      if(d_overlap % 2 != 0 && d_overlap < 2)
        throw std::runtime_error(std::string("Overlap must be even and > 0!"));

      if(d_L < 32)
        std::cerr << "Low number of subcarriers. Increase to make frame synchronization more reliable." << std::endl;

      set_min_noutput_items(d_L); // that's what is returned with each call to work
      set_history(d_num_hist_samp+1); // The first current sample is counted to the history, so add 1
    }

    /*
     * Our virtual destructor.
     */
    frame_sync_cc_impl::~frame_sync_cc_impl()
    {}

    void
    frame_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = std::max(2*d_L+d_step_size, (d_overlap/2+2)*d_L + d_step_size);
    }

    float
    frame_sync_cc_impl::corr_coef(gr_complex *x1, gr_complex *x2, gr_complex *a1)
    {
      // NOTE: This calculates the dot product of two vectors and divides it by the average power such that the result lies in [0,1]

      // results for auto- and crosscorrelation
      gr_complex xcorr = 0;
      gr_complex acorr = 0;

      // the dot prods
      volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, x1, x2, d_L);
      volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, a1, a1, d_L*2);

      //std::cout << "xcorr: " << xcorr << ". acorr: " << acorr << std::endl;
      //std::cout << "calc corr between " << x1->real() << " and " << x2->real() << std::endl;

      // acorr is calculated over two symbols, so scale accordingly
      return 2*abs(xcorr/acorr);
    }

    int
    frame_sync_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        if( ninput_items[0] < 2*d_L)
          throw std::runtime_error(std::string("Not enough input items"));

        gr_complex *in = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        //std::cout << "\ncurrent input: " << in[0].real() << std::endl;
        //std::cout << "current input buffer: ";
        //for(int i = 0; i < ninput_items[0]; i++)
        //  std::cout << in[i].real() << " ";
        //std::cout << std::endl;

        int samples_consumed = 0;
        int samples_returned = 0;

        // there are 4 cases to distinguish:
        // 1. no frame found, return no samples
        // 2. frame was already found and sync is assumed to be valid, only return samples
        // 3. frame found, sync validity expired, perform correlation in the next run
        // 4. newly found frame, return all samples from the beginning of the frame

        // frame start has already been detected
        if(d_sync_valid)
        {
          memcpy(out, in+d_num_hist_samp, d_L*sizeof(gr_complex));
          d_sym_ctr++;
          samples_consumed = d_L;
          samples_returned = d_L;

          // in the next call to work, the preamble is searched and should be found immediately
          if(d_sym_ctr >= d_frame_len)
          {
            d_sync_valid = false;
            d_sym_ctr=0;
            d_num_consec_frames++;
          }
        }
        // no frame has been detected, look for correlation peak between subsequent symbols
        else
        {
          // start returning samples if frame start was found
          float res = corr_coef(in+d_L*d_overlap/2+d_num_hist_samp, in+d_L*(d_overlap/2+1)+d_num_hist_samp, in+d_L*d_overlap/2+d_num_hist_samp);          
          if(res > d_threshold)
          {
            // copy the first symbol
            memcpy(out, in+d_num_hist_samp, d_L*sizeof(gr_complex));
            d_sync_valid = true;
            samples_returned = d_L;
            d_sym_ctr = 1;
            samples_consumed = d_L;
            if(!d_tracking)
              std::cout << "frame_sync_cc: SOF detected" << std::endl;
            d_tracking = true;
          }
          else
          {
            samples_consumed = d_step_size;
            //std::cout << "frame_sync_cc: No frame detected" << std::endl; 
            if(d_tracking)
            {
              std::cout << "frame_sync_cc: Synchronization lost after " << d_num_consec_frames << " consecutive frames" << std::endl;
              d_num_consec_frames = 0;
            }
            d_tracking = false;           
          }
        }

        // inform the scheduler about what has been going on...
        consume_each(samples_consumed);
        if(noutput_items < samples_returned)
          throw std::runtime_error(std::string("Output buffer too small"));

        //std::cout << "sym ctr: " << d_sym_ctr << std::endl;
        //std::cout << "consumed: " << samples_consumed << ". written: " << samples_returned << std::endl;
        return samples_returned;            
    }

  } /* namespace fbmc */
} /* namespace gr */

