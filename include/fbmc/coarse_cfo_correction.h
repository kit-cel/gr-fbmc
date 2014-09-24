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


#ifndef INCLUDED_FBMC_COARSE_CFO_CORRECTION_H
#define INCLUDED_FBMC_COARSE_CFO_CORRECTION_H

#include <fbmc/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup fbmc
     *
     */
    class FBMC_API coarse_cfo_correction : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<coarse_cfo_correction> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::coarse_cfo_correction.
       *
       * To avoid accidental use of raw pointers, fbmc::coarse_cfo_correction's
       * constructor is in a private implementation
       * class. fbmc::coarse_cfo_correction::make is the public interface for
       * creating new instances.
       */
      static sptr make(std::vector<int> channel_map);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_COARSE_CFO_CORRECTION_H */

