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


#ifndef INCLUDED_FBMC_FRAME_SYNC_CVC_H
#define INCLUDED_FBMC_FRAME_SYNC_CVC_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Searches for a preamble type specified by the 'preable' parameter and returns, if found, the aligned payload symbols
     * \ingroup fbmc
     *
     */
    class FBMC_API frame_sync_cvc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<frame_sync_cvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::frame_sync_cvc.
       *
       * To avoid accidental use of raw pointers, fbmc::frame_sync_cvc's
       * constructor is in a private implementation
       * class. fbmc::frame_sync_cvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L, int frame_len, std::string preamble, int step_size, float threshold);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_SYNC_CVC_H */

