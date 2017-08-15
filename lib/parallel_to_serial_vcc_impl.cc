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
#include "parallel_to_serial_vcc_impl.h"

namespace gr {
namespace fbmc {

parallel_to_serial_vcc::sptr parallel_to_serial_vcc::make(int len_out,
        int vlen_in, std::vector<int> channel_map) {
    return gnuradio::get_initial_sptr(
            new parallel_to_serial_vcc_impl(len_out, vlen_in, channel_map));
}

/*
 * The private constructor
 */
parallel_to_serial_vcc_impl::parallel_to_serial_vcc_impl(int len_out,
        int vlen_in, std::vector<int> channel_map) :
        gr::sync_interpolator("parallel_to_serial_vcc",
                gr::io_signature::make(1, 1, sizeof(gr_complex) * vlen_in),
                gr::io_signature::make(1, 1, sizeof(gr_complex)), len_out),
                d_len_out(len_out),
                d_vlen_in(vlen_in),
                d_channel_map(channel_map)
{
    // validity checks
    if (d_vlen_in < 1 || d_len_out < 1 || d_len_out > d_vlen_in)
        throw std::runtime_error("Invalid parameters!");
    else if (d_channel_map.size() != d_vlen_in)
        throw std::runtime_error(
                "Channel map size does not match the input vector length");
}

/*
 * Our virtual destructor.
 */
parallel_to_serial_vcc_impl::~parallel_to_serial_vcc_impl() {
}

int parallel_to_serial_vcc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
    gr_complex *in = (gr_complex *) input_items[0];
    gr_complex *out = (gr_complex *) output_items[0];

    int nout = noutput_items / d_len_out;
    for(int i = 0; i < nout; i++){
        unmap_one_symbol(out, in);
        out += d_len_out;
        in += d_vlen_in;
    }

    // Tell runtime system how many output items we produced.
    return nout * d_len_out;
}

void inline parallel_to_serial_vcc_impl::unmap_one_symbol(gr_complex* out,
        const gr_complex* in) {
    // Extract d_len_out samples out of the input vector, discard the rest (should be zeros anyway)
    for (int i = 0; i < d_vlen_in; i++) {
        if (d_channel_map[i] == 0)
            in++;
        else
            *out++ = *in++;
    }
}

} /* namespace fbmc */
} /* namespace gr */

