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
#include "frame_generator_vcvc_impl.h"

#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    frame_generator_vcvc::sptr
    frame_generator_vcvc::make(int sym_len, int num_payload, int inverse, int num_overlap, int num_sync)
    {
      return gnuradio::get_initial_sptr
        (new frame_generator_vcvc_impl(sym_len, num_payload, inverse, num_overlap, num_sync));
    }

    /*
     * The private constructor
     */
    frame_generator_vcvc_impl::frame_generator_vcvc_impl(int sym_len, int num_payload, int inverse, int num_overlap, int num_sync)
      : gr::block("frame_generator_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*sym_len)),
              d_sym_len(sym_len),
              d_num_payload(num_payload),
              d_num_overlap(num_overlap),
              d_num_sync(num_sync),
              d_payload_sym_ctr(0),
              d_dropped_sym_ctr(0),
              d_inverse(inverse)
    {
      // inverse has to be either 0 (insert zeros) or 1 (remove them)
      if(d_inverse != 0 && d_inverse != 1)
        throw std::runtime_error(
            std::string("inverse has to be either 0 or 1"));

      // at the moment, only an overlap of 4 is supported
      //if(d_num_overlap != 4)
      //	throw std::runtime_error(std::string("overlap has to be 4"));

      // the number of payload symbols must be >= 1
      if(d_num_payload < 1)
        throw std::runtime_error(
            std::string("number of payload symbols must be > 0"));

      // the frame length has to be a multiple of 4 because of the periodicity of the beta matrix
      d_num_frame = d_num_payload + d_num_sync + 2 * d_num_overlap;
      if(d_num_frame % 4 != 0)
        throw std::runtime_error(
            std::string(
                "frame length must be a a multiple of 4 because of the periodicity beta matrix"));

      // the block's output buffer must be able to hold at least one symbol + the zero and sync symbols
      set_output_multiple(d_num_sync + 2 * d_num_overlap + 1);
    }

    /*
     * Our virtual destructor.
     */
    frame_generator_vcvc_impl::~frame_generator_vcvc_impl()
    {
    }

    void
    frame_generator_vcvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = 1;
    }

    inline void
    frame_generator_vcvc_impl::finish_one_symbol(gr_complex* outbuf,
                                                 const gr_complex* inbuf,
                                                 const int symbol_length)
    {
      memcpy(outbuf, inbuf, sizeof(gr_complex) * symbol_length);
    }

    inline void
    frame_generator_vcvc_impl::insert_placeholder_symbols(
        gr_complex* outbuf, const int symbol_length, const int num_symbols)
    {
      int nplaceholder_symbols = symbol_length * num_symbols;
      memset(outbuf, 0, sizeof(gr_complex) * nplaceholder_symbols);
    }

    int
    frame_generator_vcvc_impl::consume_one_forward_symbol(
        gr_complex* outbuf, const gr_complex* inbuf, const int symbol_length)
    {
      int produced_items = 0;
      if(d_payload_sym_ctr == 0){ // If we are at the start of the frame, insert placeholder symbols for a preamble
        // insert preamble and num_overlap zero symbols
        insert_placeholder_symbols(outbuf, symbol_length, d_num_sync);
        // shift output pointer
        outbuf += symbol_length * d_num_sync;
        // increase output items
        produced_items += d_num_sync;
      }

      // insert the payload symbol
      finish_one_symbol(outbuf, inbuf, symbol_length);
      // shift output pointer
      outbuf += symbol_length;
      // increase output items
      produced_items += 1;
      // increase payload symbol counter
      d_payload_sym_ctr += 1;

      // If we are at the end, insert num_overlap zero symbols to allow for filter settling
      if(d_payload_sym_ctr == d_num_payload){ // the current input buffer contains the last payload symbol in the frame
        // append zero symbols
        insert_placeholder_symbols(outbuf, symbol_length, d_num_overlap);
        // shift out buffer pointer
        outbuf += symbol_length * d_num_overlap;
        // increase output items
        produced_items += d_num_overlap;
        // reset counter
        d_payload_sym_ctr = 0;
      }

      return produced_items; // max return value == d_num_sync ( > d_num_overlap)
    }

    int
    frame_generator_vcvc_impl::consume_one_reverse_symbol(
        gr_complex* outbuf, const gr_complex* inbuf, const int symbol_length)
    {
      int produced_items = 0;
      // remove zero symbols and overlap symbols if we are at the start of a frame
      if(d_payload_sym_ctr == 0){
        int num_sym_to_drop = 2 * d_num_overlap + d_num_sync; // this includes the whole preamble and the payload overlap
        if(d_dropped_sym_ctr < num_sym_to_drop){ // there are still zero/sync symbols to drop
          // drop the incoming symbol by doing nothing
          // increase dropped symbol counter
          d_dropped_sym_ctr += 1;
        }
        else // all zero/sync symbols for this frame have been dropped, copy payload from input to output
        {
          // copy first payload symbol of frame to the output buffer
          finish_one_symbol(outbuf, inbuf, symbol_length);
          // increase payload symbol counter
          d_payload_sym_ctr += 1;
          // increase number of output items
          produced_items += 1;
        }
      }
      else{
        // copy symbol to output and reset counter if needed
        finish_one_symbol(outbuf, inbuf, d_sym_len);
        // increase payload symbol counter and wrap if needed
        d_payload_sym_ctr += 1;
        // increase output items
        produced_items += 1;
        if(d_payload_sym_ctr == d_num_payload){
          d_payload_sym_ctr = 0;
          // reset counter for dropped symbols
          d_dropped_sym_ctr = 0;
        }
      }
      return produced_items; // will either return 1 or 0
    }

    int
    frame_generator_vcvc_impl::general_work(
        int noutput_items, gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      gr_complex *in = (gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int navailable_items = ninput_items[0];

      if(navailable_items < 1)
        throw std::runtime_error(
            std::string("work was called but input buffer is empty"));

      if(noutput_items < d_num_overlap * 2 + d_num_sync + 1)
        throw std::runtime_error(std::string("output buffer too small"));

      // The general frame structure is: || sync | zeros(overlap) | payload | zeros(overlap) || ...
      int produced_items = 0;
      int consumed_items = 0;

      for(int i = 0; i < navailable_items && produced_items < noutput_items - d_num_sync; i++){
        // check if we are inserting or removing zero/sync symbols
        if(d_inverse){ // reverse operation
          int iter_produced = consume_one_reverse_symbol(out, in, d_sym_len);
          out += iter_produced * d_sym_len;
          in += d_sym_len;
          consumed_items += 1;
          produced_items += iter_produced;
        }
        else{ // forward operation
          // consume one input vector and update work var's accordingly
          int iter_produced = consume_one_forward_symbol(out, in, d_sym_len);
          out += iter_produced * d_sym_len;
          in += d_sym_len;
          consumed_items += 1;
          produced_items += iter_produced;
        }
      }

      // Tell runtime system how many output items we produced and consumed.
      consume_each(consumed_items);
      return produced_items;
    }

  } /* namespace fbmc */
} /* namespace gr */

