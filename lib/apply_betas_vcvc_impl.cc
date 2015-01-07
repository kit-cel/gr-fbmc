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
#include "apply_betas_vcvc_impl.h"

#include <volk/volk.h>

namespace gr {
  namespace fbmc {

    apply_betas_vcvc::sptr
    apply_betas_vcvc::make(int L, int inverse)
    {
      return gnuradio::get_initial_sptr(new apply_betas_vcvc_impl(L, inverse));
    }

    /*
     * The private constructor
     */
    apply_betas_vcvc_impl::apply_betas_vcvc_impl(int L, int inverse) :
        gr::sync_block("apply_betas_vcvc",
                       gr::io_signature::make(1, 1, sizeof(gr_complex) * L),
                       gr::io_signature::make(1, 1, sizeof(gr_complex) * L)),
                       d_L(L),
                       d_sym_ctr(0),
                       d_inverse(inverse)
    {
      if(!(d_inverse == 0 || d_inverse == 1))
        throw std::runtime_error("inverse has to be 0 or 1");

      // define the minimum set of betas
      // frames that exceed the dimension of this set can be processed by periodic continuation of this matrix
      // basically this is the result of overlaying j**(l+k) and beta[1::2,1::2] = -beta[1::2,1::2]
      gr_complex beta[4][4] =
        {
              {gr_complex(1, 0), gr_complex(0, 1), gr_complex(-1, 0), gr_complex(0, -1)},
              {gr_complex(0, 1), gr_complex(1, 0), gr_complex(0, -1), gr_complex(-1, 0)},
              {gr_complex(-1, 0), gr_complex(0, -1), gr_complex(1, 0), gr_complex(0, 1)},
              {gr_complex(0, -1), gr_complex(-1, 0), gr_complex(0, 1), gr_complex(1, 0)}};

      // was part of old setup. Used for easier transformation into new optimized form.
      // set up the beta matrix
      std::vector<std::vector<std::vector<gr_complex> > > betas; // 3D matrix holding the beta values
      betas.resize(2); // index 0: forward, 1: reverse
      betas[0].resize(4);
      betas[1].resize(4);
      for(int i = 0; i < 4; i++){
        betas[0][i].resize(4);
        betas[1][i].resize(4);
      }
      for(int m = 0; m < 4; m++){
        for(int k = 0; k < 4; k++){
          betas[0][m][k] = beta[m][k];
          betas[1][m][k] = std::conj(beta[m][k]);
        }
      }

      // actually used arrays.
      d_betas_al.resize(2); // forward and backward
      d_betas_al[0].resize(4); // influence length
      d_betas_al[1].resize(4); // influence length

      for(int i = 0; i < 2; i++){
        for(int k = 0; k < 4; k++){
          d_betas_al[i][k] = (gr_complex*) volk_malloc(sizeof(gr_complex) * L,
                                                       volk_get_alignment());
          for(int l = 0; l < L; l++){
            d_betas_al[i][k][l] = betas[i][l % 4][k];
          }
        }
      }
    }
    /*
     * Our virtual destructor.
     */
    apply_betas_vcvc_impl::~apply_betas_vcvc_impl()
    {
    }

    int
    apply_betas_vcvc_impl::work(int noutput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for(int i = 0; i < noutput_items; i++){
        apply_betas_to_one_vector(out, in);
        out += d_L;
        in += d_L;
      }

      return noutput_items;
    }

    int
    apply_betas_vcvc_impl::apply_betas_to_one_vector(gr_complex* out,
                                                     const gr_complex* in)
    {
      volk_32fc_x2_multiply_32fc(out, in, d_betas_al[d_inverse][d_sym_ctr],
                                 d_L);
      // update symbol counter
      d_sym_ctr = (d_sym_ctr + 1) % 4;
    }

  } /* namespace fbmc */
} /* namespace gr */

