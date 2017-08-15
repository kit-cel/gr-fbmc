/* -*- c++ -*- */
/* 
 * Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT)..
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


#ifndef INCLUDED_FBMC_OUTPUT_COMMUTATOR_VCC_H
#define INCLUDED_FBMC_OUTPUT_COMMUTATOR_VCC_H

#include <fbmc/api.h>
#include <gnuradio/sync_interpolator.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Converts the incoming vector stream into a serial stream.
     * Adds the first and second half of the vector such that if a vector of length L comes in, L/2 single complex samples are produced.
     * \ingroup fbmc
     *
     */
    class FBMC_API output_commutator_vcc : virtual public gr::sync_interpolator
    {
     public:
      typedef boost::shared_ptr<output_commutator_vcc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::output_commutator_vcc.
       *
       * To avoid accidental use of raw pointers, fbmc::output_commutator_vcc's
       * constructor is in a private implementation
       * class. fbmc::output_commutator_vcc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_OUTPUT_COMMUTATOR_VCC_H */

