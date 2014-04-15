/* -*- c++ -*- */
/* 
 * Copyright 2014  Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
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


#ifndef INCLUDED_FBMC_APPLY_BETAS_VCVC_H
#define INCLUDED_FBMC_APPLY_BETAS_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief Applies the necessary phase shifts to the subcarrier symbols. Operates on one symbol (L subcarriers) at a time.
     * Set inverse to 0 in the TX path, 1 is used for RX
     * \ingroup fbmc
     *
     */
    class FBMC_API apply_betas_vcvc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<apply_betas_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::apply_betas_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::apply_betas_vcvc's
       * constructor is in a private implementation
       * class. fbmc::apply_betas_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int L, int inverse);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_APPLY_BETAS_VCVC_H */

