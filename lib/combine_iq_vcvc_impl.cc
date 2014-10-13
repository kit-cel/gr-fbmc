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
#include "combine_iq_vcvc_impl.h"

#include <volk/volk.h>

namespace gr {
namespace fbmc {

combine_iq_vcvc::sptr combine_iq_vcvc::make(int L) {
    return gnuradio::get_initial_sptr(new combine_iq_vcvc_impl(L));
}

/*
 * The private constructor
 */
combine_iq_vcvc_impl::combine_iq_vcvc_impl(int L) :
        gr::sync_decimator("combine_iq_vcvc",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(1, 1, sizeof(gr_complex)), 2), d_L(L) {
    set_output_multiple(L);
    d_inphase = (float*) volk_malloc(sizeof(float) * d_L, volk_get_alignment());
    d_quadrature = (float*) volk_malloc(sizeof(float) * d_L, volk_get_alignment());
    d_dump = (float*) volk_malloc(sizeof(float) * d_L, volk_get_alignment());
}

/*
 * Our virtual destructor.
 */
combine_iq_vcvc_impl::~combine_iq_vcvc_impl() {
    volk_free(d_inphase);
    volk_free(d_quadrature);
    volk_free(d_dump);
}

int combine_iq_vcvc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
    const gr_complex *in = (const gr_complex *) input_items[0];
    gr_complex *out = (gr_complex *) output_items[0];

    if (noutput_items < d_L)
        throw std::runtime_error("Output buffer too small");

    int num_vectors = noutput_items / d_L;
    for(int i = 0; i < num_vectors; i++){
        combine_vectors(out, in, d_L);
        in += 2 * d_L;
        out += d_L;
    }

    // Tell runtime system how many output items we produced.
    return d_L * num_vectors;
}

void
combine_iq_vcvc_impl::combine_vectors(gr_complex* out,
        const gr_complex* in, int num) {
    volk_32fc_deinterleave_32f_x2_u(d_inphase, d_dump, in, num);
    volk_32fc_deinterleave_32f_x2_u(d_quadrature, d_dump, in + num, num);
    volk_32f_x2_interleave_32fc(out, d_inphase, d_quadrature, num);
}

} /* namespace fbmc */
} /* namespace gr */

