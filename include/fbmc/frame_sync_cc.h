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


#ifndef INCLUDED_FBMC_FRAME_SYNC_CC_H
#define INCLUDED_FBMC_FRAME_SYNC_CC_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Searches for a preamble symbol specified by the 'preamble_sym' parameter and returns, if found, the aligned frame
     * \ingroup fbmc
     *
     */
    class FBMC_API frame_sync_cc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<frame_sync_cc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::frame_sync_cc.
       *
       * To avoid accidental use of raw pointers, fbmc::frame_sync_cc's
       * constructor is in a private implementation
       * class. fbmc::frame_sync_cc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L, int frame_len, std::vector<gr_complex> preamble_sym, int step_size, float threshold);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_SYNC_CC_H */

