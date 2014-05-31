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
    frame_sync_cvc::make(int L, int frame_len, std::string preamble, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new frame_sync_cvc_impl(L, frame_len, preamble, step_size, threshold));
    }

    /*
     * The private constructor
     */
    frame_sync_cvc_impl::frame_sync_cvc_impl(int L, int frame_len, std::string preamble, int step_size, float threshold)
      : gr::block("frame_sync_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, L*sizeof(gr_complex))),
                d_L(L),
                d_frame_len(frame_len),
                d_preamble(preamble),
                d_step_size(step_size),
                d_threshold(threshold),
                d_frame_found(false),
                d_sym_ctr(0)
    {
      buf = new boost::circular_buffer<gr_complex>(d_L);

      if(d_threshold <= 0 || d_threshold >= 1)
        throw std::runtime_error(std::string("Threshold must be between (0,1)")); 

      if(d_step_size > d_L)
        throw std::runtime_error(std::string("Step size must be smaller or equal to the symbol length"));
    }

    /*
     * Our virtual destructor.
     */
    frame_sync_cvc_impl::~frame_sync_cvc_impl()
    {
      delete buf;
    }

    void
    frame_sync_cvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = 2*d_L+d_step_size;
    }

    int
    frame_sync_cvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        if( ninput_items[0] < 2*d_L)
          throw std::runtime_error(std::string("Not enough input items"));

        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        int samples_consumed = 0;
        int items_written = 0;

        // fill the buffer at startup and return
        if(buf->size() < buf->capacity())
        {
          while(buf->size() < buf->capacity()) 
            buf->push_front(in[samples_consumed++]);  
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
          memcpy(out, in, d_L);
          d_sym_ctr++;
          samples_consumed += d_L;
          items_written = 1;
          
          if(d_sym_ctr > d_frame_len-1)// last symbol of the frame, check if the frame sync is still valid
          {
            d_sym_ctr = 0; // reset the counter
            gr_complex acorr = 1;
            gr_complex xcorr = 0;
            volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, in, in+d_L, d_L);
            volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, in, in, d_L*2);
            if(2*abs(xcorr/acorr) < d_threshold) // sync not valid anymore
              d_frame_found = false;
          }
        }
        // no frame has been detected, look for correlation peak between subsequent symbols
        else
        {
          // proceed by d_step_size samples and calculate correlation
          for(int i=0; i<d_step_size; i++)
            buf->push_front(in[i]);          
          samples_consumed += d_step_size;

          // perform correlation with VOLK
          gr_complex acorr = 1;
          gr_complex xcorr = 0;
          volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, &buf->at(0), &in[d_step_size], d_L);
          volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, &in[d_step_size], &in[d_step_size], d_L*2);
          // start returning samples if frame start was found
          if(2*abs(xcorr/acorr) > d_threshold)
          {
            memcpy(out, in+d_step_size, d_L);
            d_frame_found = true;
            items_written = 1;
            d_sym_ctr++;
            samples_consumed += d_L;
          }
        }

        // inform the scheduler about what has been going on...
        consume_each(samples_consumed);
        if(noutput_items < items_written)
          throw std::runtime_error(std::string("Output buffer too small"));
        return items_written;            
    }

  } /* namespace fbmc */
} /* namespace gr */

