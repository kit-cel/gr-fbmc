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


#ifndef INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_H
#define INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_H

#include <fbmc/api.h>
#include <gnuradio/sync_interpolator.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Takes a vector of samples, removes the last (vlen_in-len_out) samples and streams the rest to the output
     * \ingroup fbmc
     *
     */
    class FBMC_API parallel_to_serial_vcc : virtual public gr::sync_interpolator
    {
     public:
      typedef boost::shared_ptr<parallel_to_serial_vcc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::parallel_to_serial_vcc.
       *
       * To avoid accidental use of raw pointers, fbmc::parallel_to_serial_vcc's
       * constructor is in a private implementation
       * class. fbmc::parallel_to_serial_vcc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int len_out, int vlen_in, std::vector<int> channel_map);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_PARALLEL_TO_SERIAL_VCC_H */

