/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_FBMC_SMT_KERNEL_H
#define INCLUDED_FBMC_SMT_KERNEL_H

#include <fbmc/api.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Interface class for all SMT FBMC filters
     *
     */
    class FBMC_API smt_kernel
    {
    public:
      smt_kernel(const std::vector<float> &taps, int L);
      ~smt_kernel();

      virtual int generic_work(gr_complex* out, const gr_complex* in, int noutput_items) = 0;

      int L(){return d_L;};
      int overlap(){return d_overlap;};
      virtual int fft_size() = 0;
      std::vector<float> taps(){return d_taps;};
    protected:
      int d_L;
      int d_overlap;
      std::vector<float> d_taps;
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SMT_KERNEL_H */

