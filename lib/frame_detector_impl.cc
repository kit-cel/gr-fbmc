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
#include "frame_detector_impl.h"
#include <algorithm>

namespace gr {
  namespace fbmc {

    frame_detector::sptr
    frame_detector::make(int frame_len)
    {
      return gnuradio::get_initial_sptr
        (new frame_detector_impl(frame_len));
    }

    /*
     * The private constructor
     */
    frame_detector_impl::frame_detector_impl(int frame_len)
      : gr::sync_block("frame_detector",
              gr::io_signature::make2(2, 2, sizeof(float), sizeof(gr_complex)),
              gr::io_signature::make2(2, 2, sizeof(float), sizeof(gr_complex))),
              d_frame_len(frame_len)
    {
      set_output_multiple(2 * d_frame_len);  
    }

    /*
     * Our virtual destructor.
     */
    frame_detector_impl::~frame_detector_impl()
    {
    }
    
    int 
    frame_detector_impl::argmax(float* start, int len)
    {
      int idx = 0;
      float max = *start;
      for(int i=1; i < len; i++)
      {
        if(*(start+i) > max)
        { 
          max = *(start+i); 
          idx = i; 
        }
      }
      return idx;
    }

    int
    frame_detector_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      float *corr_in = (float *) input_items[0];
      gr_complex *sig_in = (gr_complex *) input_items[1];
      float *corr_out = (float *) output_items[0];
      gr_complex *sig_out = (gr_complex *) output_items[1];

      int nframes = noutput_items / d_frame_len - 1;
      
      for(int i = 0; i < nframes; i++)
      {
        int offset = argmax(corr_in + i * d_frame_len, d_frame_len);
        memcpy(sig_out + i * d_frame_len, sig_in + i * d_frame_len + offset, sizeof(gr_complex) * d_frame_len);
        memcpy(corr_out + i * d_frame_len, corr_in + i * d_frame_len, sizeof(float) * d_frame_len);
        add_item_tag(1, nitems_written(1) + i * d_frame_len + offset, pmt::intern("peak"), pmt::PMT_NIL);
      }

      return nframes * d_frame_len;
    }

  } /* namespace fbmc */
} /* namespace gr */

