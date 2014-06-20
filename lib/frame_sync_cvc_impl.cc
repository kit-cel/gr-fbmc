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
#include "frame_sync_cvc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    frame_sync_cvc::sptr
    frame_sync_cvc::make(int L, int frame_len, int overlap, std::string preamble, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new frame_sync_cvc_impl(L, frame_len, overlap, preamble, step_size, threshold));
    }

    /*
     * The private constructor
     */
    frame_sync_cvc_impl::frame_sync_cvc_impl(int L, int frame_len, int overlap, std::string preamble, int step_size, float threshold)
      : gr::block("frame_sync_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, L*sizeof(gr_complex))),
                d_L(L),
                d_frame_len(frame_len),
                d_overlap(overlap),
                d_preamble(preamble),
                d_step_size(step_size),
                d_threshold(threshold),
                d_frame_found(false),
                d_sym_ctr(0)
    {
      if(d_threshold <= 0 || d_threshold >= 1)
        throw std::runtime_error(std::string("Threshold must be between (0,1)")); 

      if(d_step_size > d_L)
        throw std::runtime_error(std::string("Step size must be smaller than or equal to the symbol length"));

      if(d_preamble != "IAM")
        throw std::runtime_error(std::string("Only IAM is supported"));

      if(d_overlap % 2 != 0 && d_overlap > 0)
        throw std::runtime_error(std::string("Overlap must be even and > 0!"));

      if(d_L < 32)
        std::cerr << "Low number of subcarriers. Increase to make frame synchronization more reliable." << std::endl;

      d_buf = boost::circular_buffer<gr_complex>(d_L);
      d_num_hist_sym = d_overlap/2;
    }

    /*
     * Our virtual destructor.
     */
    frame_sync_cvc_impl::~frame_sync_cvc_impl()
    {}

    void
    frame_sync_cvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = 2*d_L+d_step_size;
    }

    float
    frame_sync_cvc_impl::corr_coef(gr_complex *x1, gr_complex *x2, gr_complex *a1)
    {
      // NOTE: This calculates the dot product of two vectors and divides it by the average power such that the result lies in [0,1]

      // results for auto- and crosscorrelation
      gr_complex xcorr = 0;
      gr_complex acorr = 0;

      // the dot prods
      volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, x1, x2, d_L);
      volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, a1, a1, d_L*2);

      //std::cout << "xcorr: " << xcorr << ". acorr: " << acorr << std::endl;

      // acorr is calculated over two symbols, so scale accordingly
      return 2*abs(xcorr/acorr);
    }

    int
    frame_sync_cvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        if( ninput_items[0] < 2*d_L)
          throw std::runtime_error(std::string("Not enough input items"));

        gr_complex *in = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        int samples_consumed = 0;
        int items_written = 0;

        // fill the buffer at startup and return
        if(d_buf.size() < d_buf.capacity())
        {
          //std::cout << "Startup, fill buffer and return." << std::endl;
          while(d_buf.size() < d_buf.capacity()) 
            d_buf.push_back(in[samples_consumed++]);  
            
          consume_each(samples_consumed);
          return items_written;
        }

        // there are 3 cases to distinguish:
        // 1. no frame found 
        // 2. frame found and sync is assumed to be valid, only return samples
        // 3. frame found, check if sync still valid

        // IMPORTANT NOTE: the frame start corresponds to the 2 equal subsequent symbols, not the begin/end of the payload!

        // frame start has already been detected
        if(d_frame_found)
        {
          //std::cout << "Frame sync assumed valid, copy symbol to output." << std::endl;
          memcpy(out, in, d_L);
          d_sym_ctr++;
          samples_consumed += d_L;
          items_written = 1;
          
          if(d_sym_ctr > d_frame_len-1)// last symbol of the frame, check if the frame sync is still valid
          {
            //std::cout << "\tLast symbol in frame reached. ";
            d_sym_ctr = 0; // reset the counter
            float res = corr_coef(in, in+d_L, in);
            //std::cout << "Corr: " << res;
            if(res < d_threshold) // sync not valid anymore
            {
              d_frame_found = false;
              d_buf.clear();
              std::cout << "frame_sync_cvc: Frame sync lost" << std::endl;
            }
            //else
              //std::cout << ". Frame sync still valid.";
          }
        }
        // no frame has been detected, look for correlation peak between subsequent symbols
        else
        {
          //std::cout << "Looking for frame. ";
          // proceed by d_step_size samples and calculate correlation
          for(int i=0; i<d_step_size; i++)
          {
            //std::cout << "buf pushback: " << in[i] << std::endl;
            d_buf.push_back (in[i]);    
          }      
          samples_consumed += d_step_size;

          // start returning samples if frame start was found
          d_buf.linearize();
          float res = corr_coef(&d_buf[0], in+d_step_size, in+d_step_size);
          //std::cout << "Corr: " << res;
          if(res > d_threshold)
          {
            std::cout << "frame_sync_cvc: Found start of frame." << std::endl;
            memcpy(out, in+d_step_size, d_L);
            d_frame_found = true;
            items_written = 1;
            d_sym_ctr++;
            samples_consumed += d_L;
          }
          //else
          //  std::cout << ". No start of frame detected." << std::endl;
        }

        // inform the scheduler about what has been going on...
        consume_each(samples_consumed);
        if(noutput_items < items_written)
          throw std::runtime_error(std::string("Output buffer too small"));

        //std::cout << "consumed: " << samples_consumed << ". written: " << items_written << std::endl;
        return items_written;            
    }

  } /* namespace fbmc */
} /* namespace gr */

