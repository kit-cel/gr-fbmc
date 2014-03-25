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


#ifndef INCLUDED_FBMC_POLYPHASE_FILTERBANK_VCVC_H
#define INCLUDED_FBMC_POLYPHASE_FILTERBANK_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Uses a polyphase approach to filter the input sequence with the prototype filter
     * \ingroup fbmc
     *
     */
    class FBMC_API polyphase_filterbank_vcvc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<polyphase_filterbank_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::polyphase_filterbank_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::polyphase_filterbank_vcvc's
       * constructor is in a private implementation
       * class. fbmc::polyphase_filterbank_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(std::vector<gr_complex> taps, int L);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_POLYPHASE_FILTERBANK_VCVC_H */

