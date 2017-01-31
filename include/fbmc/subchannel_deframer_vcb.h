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


#ifndef INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_H
#define INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup fbmc
     *
     */
    class FBMC_API subchannel_deframer_vcb : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<subchannel_deframer_vcb> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::subchannel_deframer_vcb.
       *
       * To avoid accidental use of raw pointers, fbmc::subchannel_deframer_vcb's
       * constructor is in a private implementation
       * class. fbmc::subchannel_deframer_vcb::make is the public interface for
       * creating new instances.
       */
      static sptr make(int subcarriers, int bands, int guard, float threshold, std::vector<gr_complex> preamble, int symbols, std::vector<int> pilot_carriers, int pilot_timestep);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SUBCHANNEL_DEFRAMER_VCB_H */

