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


#ifndef INCLUDED_FBMC_FRAME_GENERATOR_VCVC_H
#define INCLUDED_FBMC_FRAME_GENERATOR_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Creates a frame structure be pre- and appending zero symbols that can be used e.g. for preambles and allows for filter settling
     * sym_len: number of carriers per symbol
     * num_payload: number of actual payload symbols per frame
     * inverse: if inverse=1, the frame structure is removed and the payload is returned.
     * num_overlap: number of overlapping symbols
     * num_sync: number of sync symbols per frame
	 * 
     * \ingroup fbmc
     *
     */
    class FBMC_API frame_generator_vcvc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<frame_generator_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::frame_generator_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::frame_generator_vcvc's
       * constructor is in a private implementation
       * class. fbmc::frame_generator_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int sym_len, int num_payload, int inverse, int num_overlap, int num_sync);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_FRAME_GENERATOR_VCVC_H */

