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


#ifndef INCLUDED_FBMC_SERIALIZE_IQ_VCVC_H
#define INCLUDED_FBMC_SERIALIZE_IQ_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_interpolator.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Splits a complex number into its real and imaginary part.
     * Complex numbers come in as vector of length L and are split up into two vectors, each of length L
     * \ingroup fbmc
     *
     */
    class FBMC_API serialize_iq_vcvc : virtual public gr::sync_interpolator
    {
     public:
      typedef boost::shared_ptr<serialize_iq_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::serialize_iq_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::serialize_iq_vcvc's
       * constructor is in a private implementation
       * class. fbmc::serialize_iq_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_SERIALIZE_IQ_VCVC_H */

