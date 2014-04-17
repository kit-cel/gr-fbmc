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


#ifndef INCLUDED_FBMC_SYMBOLS_TO_BITS_CB_H
#define INCLUDED_FBMC_SYMBOLS_TO_BITS_CB_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/digital/constellation.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Maps the incoming samples to the closest constellation point (4-QAM assumed)
     * \ingroup fbmc
     *
     */
    class FBMC_API symbols_to_bits_cb : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<symbols_to_bits_cb> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::symbols_to_bits_cb.
       *
       * To avoid accidental use of raw pointers, fbmc::symbols_to_bits_cb's
       * constructor is in a private implementation
       * class. fbmc::symbols_to_bits_cb::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SYMBOLS_TO_BITS_CB_H */

