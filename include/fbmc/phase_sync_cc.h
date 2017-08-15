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


#ifndef INCLUDED_FBMC_PHASE_SYNC_CC_H
#define INCLUDED_FBMC_PHASE_SYNC_CC_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup fbmc
     *
     */
    class FBMC_API phase_sync_cc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<phase_sync_cc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::phase_sync_cc.
       *
       * To avoid accidental use of raw pointers, fbmc::phase_sync_cc's
       * constructor is in a private implementation
       * class. fbmc::phase_sync_cc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int search_window, std::vector<gr_complex> preamble, float threshold, int L, int nsym_frame, std::vector<float> taps);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PHASE_SYNC_CC_H */

