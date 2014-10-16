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


#ifndef INCLUDED_FBMC_SMT_FILTERBANK_KERNEL_H
#define INCLUDED_FBMC_SMT_FILTERBANK_KERNEL_H

#include <fbmc/api.h>
#include <gnuradio/filter/filterbank.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief actual kernel for SMT Filterbank
     *
     */
    class FBMC_API smt_filterbank_kernel: public filter::kernel::filterbank
    {
    public:
      smt_filterbank_kernel(std::vector<float> &taps, int L);
      ~smt_filterbank_kernel();

      int L(){return d_L;};
      std::vector<std::vector<float> > taps(){return filterbank::taps();};

      int generic_work(gr_complex* out, const gr_complex* in, int noutput_items);

    private:
      int d_L;
      std::vector<std::vector<float> > d_prototype_taps;
      void set_taps(std::vector<float> &taps);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SMT_FILTERBANK_KERNEL_H */

