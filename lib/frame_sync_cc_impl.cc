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
    frame_sync_cc::make(int L, int frame_len, std::vector<gr_complex> preamble_sym, int step_size, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new frame_sync_cc_impl(L, frame_len, preamble_sym, step_size, threshold));
    }

    /*
     * The private constructor
     */
    frame_sync_cc_impl::frame_sync_cc_impl(int L, int frame_len, std::vector<gr_complex> preamble_sym, int step_size, float threshold)
      : gr::block("frame_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
                d_L(L),
                d_frame_len(frame_len),
                d_preamble_sym(preamble_sym),
                d_step_size(step_size),
                d_threshold(threshold),
                d_sync_valid(false),
                d_tracking(false),
                d_num_consec_frames(0),
                d_sym_ctr(0),
                d_f_off(0),
                d_phi_off(0)
    {
      dbg_fp = fopen("fs_lc.bin", "wb");
      dbg_fp2 = fopen("fs_in.bin", "wb");
      dbg_fp3 = fopen("fs_rc.bin", "wb");

      if(d_step_size != 1)
        throw std::runtime_error(std::string("Step size must be 1 at the moment because there is no sensible use implemented yet"));

      if(d_threshold <= 0 || d_threshold >= 1)
        throw std::runtime_error(std::string("Threshold must be between (0,1)")); 

      if(d_step_size > d_L)
        throw std::runtime_error(std::string("Step size must be smaller than or equal to the symbol length"));

      if(d_L < 32)
        std::cerr << "Low number of subcarriers. Increase to make frame synchronization more reliable." << std::endl;

      volk_32fc_x2_conjugate_dot_prod_32fc(&d_preamble_energy, &d_preamble_sym[0], &d_preamble_sym[0], d_preamble_sym.size());
      d_preamble_energy = abs(d_preamble_energy);

      set_min_noutput_items(d_L); // that's what is returned with each call to work
    }

    /*
     * Our virtual destructor.
     */
    frame_sync_cc_impl::~frame_sync_cc_impl()
    {
      fclose(dbg_fp);
      fclose(dbg_fp2);
      fclose(dbg_fp3);
    }

    void
    frame_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        // this is a few samples more than we actually need
        ninput_items_required[0] = std::max(2*d_L + d_step_size, int(d_preamble_sym.size()) + d_step_size);
    }

    float 
    frame_sync_cc_impl::estimate_f_off(gr_complex corr_val)
    { 
      //std::cout << "corr:" << corr_val << ", f_off:" << 1.0/(2*M_PI*d_L)*arg(-corr_val) << std::endl;
      return -1.0/(2*M_PI*d_L)*arg(-corr_val); 
    }

    float 
    frame_sync_cc_impl::estimate_phi_off(gr_complex* rx_pil)
    { 
      //std::cout << "phi_off:" << arg(*rx_pil/d_ref_pil) << std::endl;
      return arg(*rx_pil); 
    } 

    gr_complex
    frame_sync_cc_impl::fixed_lag_corr(gr_complex *x1, gr_complex *x2, gr_complex *a1)
    {
      // NOTE: This calculates the dot product of two vectors and divides it by the average power such that the result lies in [0,1]

      // results for auto- and crosscorrelation
      gr_complex xcorr = 0;
      gr_complex acorr = 0;

      // the dot prods
      // std::cout << "compare:";
      // for(int i=0; i<d_L; i++)
      //   std::cout << x1[i] << " ";
      // std::cout << " with ";
      // for(int i=0; i<d_L; i++)
      //   std::cout << x2[i] << " ";
      // std::cout << std::endl;

      volk_32fc_x2_conjugate_dot_prod_32fc(&xcorr, x1, x2, d_L);
      volk_32fc_x2_conjugate_dot_prod_32fc(&acorr, a1, a1, d_L*2);

      // acorr is calculated over two symbols, so scale accordingly
      return gr_complex(2,0)*xcorr/acorr;
    }

    gr_complex
    frame_sync_cc_impl::ref_corr(gr_complex *x)
    {
      gr_complex corr = 0;
      gr_complex sig_energy = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&corr, x, &d_preamble_sym[0], d_preamble_sym.size());
      volk_32fc_x2_conjugate_dot_prod_32fc(&sig_energy, x, x, d_preamble_sym.size());
      return corr/sqrt(d_preamble_energy*sig_energy);
    }

    void
    frame_sync_cc_impl::correct_offsets(gr_complex* buf, float f_off, float phi_prev)
    {
      for(int i=0; i<d_L; i++)
        buf[i] *= exp(gr_complex(0,1)*gr_complex(-2*M_PI*f_off*i - phi_prev, 0));
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

        int samples_consumed = 0;
        int samples_returned = 0;

        /* Acquisition algorithm:
        * Step 1: Perform correlation of two subsequent symbols about 2 symbols in advance. 
        *         The peak itself is too broad for a SOF detection but the phase around the maximum is very stable and can therefore be used for CFO correction
        *         This is needed as first step because the frequency offset degrades the correlation to the reference symbol severely.
        * Step 2: Perform correlation with reference symbol on the frequency corrected input signal to obtain the exact location of the peak and therefore the SOF
        *
        * Tracking algorithm:
        * Assume the synchronization to be valid for a whole frame. 
        * At the (expected) beginning of each frame perform a fixed lag correlation to update the frequency and phase offset.
        * Then correlate with the reference symbol and confirm the validity of the expected SOF
        */


        // there are 4 cases to distinguish:
        // 1. no frame found, return no samples
        // 2. frame was already found and sync is assumed to be valid, only return samples
        // 3. frame found, sync validity expired, perform correlation in the next run
        // 4. newly found frame, start returning samples

        if(!d_tracking) // acquisition
        {
          // perform a fixed lag correlation of the next two consecutive symbols
          gr_complex lagged_corr_res = fixed_lag_corr(in, in+d_L, in);
          gr_complex ref_corr_res = ref_corr(in);
          fwrite(&lagged_corr_res, sizeof(gr_complex), 1, dbg_fp);
          fwrite(&ref_corr_res, sizeof(gr_complex), 1, dbg_fp3);
          fwrite(in, sizeof(gr_complex), 1, dbg_fp2);
          samples_consumed = 1;

        }
        // frame start has already been detected
        //if(d_sync_valid)
        //{
        //   // std::cout << "SYNC VALID" << std::endl;
        //   correct_offsets(in, d_f_off, d_phi_off);
        //   d_phi_off = fmod(d_phi_off + 2*M_PI*d_f_off*d_L, 2*M_PI);
        //   memcpy(out, in, d_L*sizeof(gr_complex));
        //   d_sym_ctr++;
        //   samples_consumed = d_L;
        //   samples_returned = d_L;

        //   // in the next call to work, the preamble is searched and should be found immediately
        //   if(d_sym_ctr >= d_frame_len)
        //   {
        //     d_sync_valid = false;
        //     d_sym_ctr=0;
        //   }
        // }
        // else if(d_tracking) // tracking
        // {
        //   // std::cout << "TRACKING" << std::endl;

        //   gr_complex corr = corr_coef(in, &d_preamble_sym[0], in+d_L*d_overlap);
        //   // std::cout << "TRA i:" << i << " corr:" << corr << std::endl;
          
        //   // std::cout << "#frame:" << d_num_consec_frames << std::endl;
        //   d_num_consec_frames++;
        //   samples_consumed = d_L;
        //   if(abs(corr) > d_threshold) // SOF found
        //   {
        //     // estimate and correct frequency and phase offsets
        //     //d_f_off = estimate_f_off(corr);
        //     //d_phi_off = fmod(estimate_phi_off(in+d_L*d_overlap) - 2*M_PI*d_f_off*d_L*d_overlap, 2*M_PI); // the estimated phase offset has to be turned back by the phase increment caused by the frequency offset
        //     //std::cout << "TRA foff(*250e3):" << d_f_off*250e3 << ", phioff:" << d_phi_off << std::endl;
        //     //correct_offsets(in, d_f_off, d_phi_off);
        //     //d_phi_off = fmod(d_phi_off + 2*M_PI*d_f_off*d_L, 2*M_PI);

        //     // copy the first symbol
        //     //memcpy(out, in, d_L*sizeof(gr_complex));
        //     //d_sym_ctr = 1;
        //     //d_sync_valid = true;            
        //     //samples_returned = d_L;
        //   }
        //   else // sync lost
        //   {
        //     // std::cout << "frame_sync_cc: Synchronization lost after " << d_num_consec_frames << " consecutive frames, back to acquisition mode" << std::endl;
        //     // d_tracking = false;
        //     // samples_returned = 0;
        //     // d_num_consec_frames = 0;
        //   }   
        // }
        // else // acquisition, proceed one sample at a time
        // {
          // std::cout << "ACQUISITION" << std::endl;
          // gr_complex corr = corr_coef(in+d_L*d_overlap, in+d_L*(d_overlap+1), in+d_L*d_overlap);    
          // //std::cout << "ACQ corr:" << corr << std::endl;      
          // if(abs(corr) > d_threshold)
          // {
          //   // estimate and correct offsets
          //   d_f_off = estimate_f_off(corr);
          //   d_phi_off = fmod(estimate_phi_off(in+d_L*d_overlap) - 2*M_PI*d_f_off*d_L*d_overlap, 2*M_PI);
          //   std::cout << "ACQ foff(*250e3):" << d_f_off*250e3 << ", phioff:" << d_phi_off << std::endl;
          //   correct_offsets(in, d_f_off, d_phi_off);
          //   d_phi_off = fmod(d_phi_off+2*M_PI*d_f_off*d_L, 2*M_PI);
          //   //std::cout << "ACQ updated phioff:" << d_phi_off << std::endl;

          //   // copy the first symbol
          //   memcpy(out, in, d_L*sizeof(gr_complex));
          //   d_sync_valid = true;
          //   samples_returned = d_L;
          //   d_sym_ctr = 1;
          //   samples_consumed = d_L;
          //   if(!d_tracking)
          //     std::cout << "frame_sync_cc: SOF detected, enter tracking mode" << std::endl;
          //   d_tracking = true;
          // }
          // else
          // {
          //   samples_consumed = d_step_size; 
          // }
              
        //}

        // inform the scheduler about what has been going on...
        //std::cout << "consumed:" << samples_consumed << ", returned:" << samples_returned << std::endl;
        consume_each(samples_consumed);
        if(noutput_items < samples_returned)
          throw std::runtime_error(std::string("Output buffer too small"));

        return samples_returned;            
    }

  } /* namespace fbmc */
} /* namespace gr */

