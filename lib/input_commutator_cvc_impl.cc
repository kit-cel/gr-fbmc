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
#include "input_commutator_cvc_impl.h"

namespace gr {
namespace fbmc {

input_commutator_cvc::sptr input_commutator_cvc::make(int L) {
    return gnuradio::get_initial_sptr(new input_commutator_cvc_impl(L));
}

/*
 * The private constructor
 */
input_commutator_cvc_impl::input_commutator_cvc_impl(int L) :
        gr::block("input_commutator_cvc",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(1, 1, sizeof(gr_complex) * L)),
                d_L(L)
{
    if (d_L < 2 || d_L % 2 != 0){
        throw std::runtime_error("L has to be even and >= 2!");
    }
    // history is needed to generate type-III polyphase components
    set_history(L / 2);
    set_output_multiple(2);
}

/*
 * Our virtual destructor.
 */
input_commutator_cvc_impl::~input_commutator_cvc_impl() {
}

void input_commutator_cvc_impl::forecast(int noutput_items,
        gr_vector_int &ninput_items_required) {
    ninput_items_required[0] = d_L;
}

int input_commutator_cvc_impl::general_work(int noutput_items,
        gr_vector_int &ninput_items, gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
    gr_complex *in = (gr_complex *) input_items[0];
    gr_complex *out = (gr_complex *) output_items[0];

    std::cout << "work call with: " << noutput_items << " items" << std::endl;

//    for(int i = 0; i < 10; i++){
//        std::cout << in[i] << ", ";
//    }
//    std::cout << std::endl;

    // one call to work produces 2 output vectors
    // the second half of each vector is equal to the first half
    // history() - 1 == (L / 2) -1 which is the necessary offset.
    int output_symbols = 1; noutput_items / 2;
    for(int i = 0; i < output_symbols; i++){
        produce_to_out_vectors(out, in, history() - 1, d_L);
        in += d_L;
        out += 2 * d_L;
    }



    // Tell runtime system how many output items we produced and consumed.
    consume_each(d_L);
    return 2;
}

void inline
input_commutator_cvc_impl::copy_to_internal_buffer(std::valarray<gr_complex>& buf, const gr_complex* in, int offset, int num)
{
    memcpy((void*) &buf[offset], (void*) in, num*sizeof(gr_complex));
}

void inline
input_commutator_cvc_impl::produce_to_out_vectors(gr_complex* out, const gr_complex* in, int offset, int num)
{
    int half = num / 2;
    for (int k = 0; k < 2; k++) {
        produce_one_symbol(out + k * num, in + k * half, half);
    }
}

void inline
input_commutator_cvc_impl::produce_one_symbol(gr_complex* out, const gr_complex* in, int num)
{
    reverse_for_polyphase_filter(out, in, num);
    memcpy(out + num, out, num * sizeof(gr_complex));
    std::cout << "one symbol";
    for(int i = 0; i < 2 * num; i++){
        std::cout << out[i] << ", ";
    }
    std::cout << std::endl;
}

void inline
input_commutator_cvc_impl::reverse_for_polyphase_filter(gr_complex* out, const gr_complex* in, int num)
{
    std::cout << "reverse: ";
    for (int l = 0; l < num; l++) {
        out[l] = in[(num - 1) - l];
//        printf("%f, ", out[l].real());
        std::cout << out[l] << ", ";
//        out[k * num + l] = buf[k * half + offset - l];
//        out[k * num + half + l] = out[k * num + l];
    }
    std::cout << std::endl;
//    printf("\n");
}

void
input_commutator_cvc_impl::print_valarray(std::valarray<gr_complex>& buf)
{
    for(int i = 0; i < buf.size(); i++){
        std::cout << buf[i].real() << ", ";
    }
    std::cout << std::endl;
}

} /* namespace fbmc */
} /* namespace gr */

