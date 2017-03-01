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


#ifndef INCLUDED_FBMC_CORRELATOR_POSTPROCESSOR_CF_H
#define INCLUDED_FBMC_CORRELATOR_POSTPROCESSOR_CF_H

#include <fbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace fbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup fbmc
     *
     */
    class FBMC_API correlator_postprocessor_cf : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<correlator_postprocessor_cf> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fbmc::correlator_postprocessor_cf.
       *
       * To avoid accidental use of raw pointers, fbmc::correlator_postprocessor_cf's
       * constructor is in a private implementation
       * class. fbmc::correlator_postprocessor_cf::make is the public interface for
       * creating new instances.
       */
      virtual void set_offset(float offset) = 0;
      static sptr make(float offset, int window_len);
    };

  } // namespace fbmc
} // namespace gr

#endif /* INCLUDED_FBMC_CORRELATOR_POSTPROCESSOR_CF_H */

