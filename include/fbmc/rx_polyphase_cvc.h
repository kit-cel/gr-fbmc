/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_FBMC_RX_POLYPHASE_CVC_H
#define INCLUDED_FBMC_RX_POLYPHASE_CVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief IOTA Filterbank combines Commutator, PFB and IFFT
     * \ingroup fbmc
     *
     */
    class FBMC_API rx_polyphase_cvc : virtual public gr::sync_decimator
    {
    public:
      typedef boost::shared_ptr<rx_polyphase_cvc> sptr;

      virtual std::vector<std::vector<float> > filterbank_taps() = 0;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::rx_polyphase_cvc.
       *
       * To avoid accidental use of raw pointers, fbmc::rx_polyphase_cvc's
       * constructor is in a private implementation
       * class. fbmc::rx_polyphase_cvc::make is the public interface for
       * creating new instances.
       */
      static sptr
      make(std::vector<float> taps, int L);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_RX_POLYPHASE_CVC_H */

