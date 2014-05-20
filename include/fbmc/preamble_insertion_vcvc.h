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


#ifndef INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_H
#define INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Inserts a preamble that can be used for channel equalization
     * \ingroup fbmc
     *
     */
    class FBMC_API preamble_insertion_vcvc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<preamble_insertion_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::preamble_insertion_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::preamble_insertion_vcvc's
       * constructor is in a private implementation
       * class. fbmc::preamble_insertion_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L, int frame_len, std::string type, int overlap);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PREAMBLE_INSERTION_VCVC_H */

