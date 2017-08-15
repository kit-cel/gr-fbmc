/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_H
#define INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup fbmc
     *
     */
    class FBMC_API channel_estimator_vcvc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<channel_estimator_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::channel_estimator_vcvc.
       *
       * To avoid accidental use of raw pointers, fbmc::channel_estimator_vcvc's
       * constructor is in a private implementation
       * class. fbmc::channel_estimator_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int frame_len, int subcarriers, int overlap, int bands, std::vector<float> taps,
                       float pilot_amp, int pilot_timestep, std::vector<int> pilot_carriers);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CHANNEL_ESTIMATOR_VCVC_H */

