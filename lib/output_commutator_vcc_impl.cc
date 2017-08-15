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
#include "output_commutator_vcc_impl.h"

#include <volk/volk.h>

namespace gr {
namespace fbmc {

output_commutator_vcc::sptr output_commutator_vcc::make(int L) {
    return gnuradio::get_initial_sptr(new output_commutator_vcc_impl(L));
}

/*
 * The private constructor
 */
output_commutator_vcc_impl::output_commutator_vcc_impl(int L) :
        gr::sync_interpolator("output_commutator_vcc",
                gr::io_signature::make(1, 1, sizeof(gr_complex) * L),
                gr::io_signature::make(1, 1, sizeof(gr_complex)), L / 2),
                d_L(L)
{
    if (d_L % 2 != 0) {
        throw std::runtime_error("vector length must be even");
    }

}

/*
 * Our virtual destructor.
 */
output_commutator_vcc_impl::~output_commutator_vcc_impl() {
}

int output_commutator_vcc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
    const gr_complex *in = (const gr_complex *) input_items[0];
    gr_complex *out = (gr_complex *) output_items[0];

    int nout = noutput_items / (d_L / 2);
    for(int i = 0; i < nout; i++){
        commutate_one_symbol(out, in);
        out += (d_L / 2);
        in += d_L;
    }

    // Tell runtime system how many output items we produced.
    return nout * d_L / 2;
}

void inline output_commutator_vcc_impl::commutate_one_symbol(gr_complex* out,
        const gr_complex* in) {
    // Add first and second half and write the result to the output buffer
    volk_32f_x2_add_32f((float*) out,(float*) in,(float*) (in + (d_L / 2)), d_L);
}

} /* namespace fbmc */
} /* namespace gr */

