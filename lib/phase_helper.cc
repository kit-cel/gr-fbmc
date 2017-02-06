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


#include "phase_helper.h"
#include <numeric>

namespace gr {
  namespace fbmc {
    phase_helper::phase_helper()
    {
      reset_angle();
    }

    phase_helper::~phase_helper() {}

    void
    phase_helper::reset_angle() {
      d_prev_angle = 0.0;
    }

    std::vector<double>
    phase_helper::linear_regression(std::vector<double> data) {
      // return vector of {a, b} with linear regression line ax+b for y data points
      std::vector<double> x;
      for(unsigned int i = 0; i < data.size(); i++) {
        x.push_back(i);
      }
      double x_mean = (data.size()-1)/2.0;
      double y_mean = std::accumulate(data.begin(), data.end(), 0.0)/data.size();
      std::for_each(x.begin(), x.end(), [&](double& d) { d -= x_mean;}); // x-x_mean
      std::for_each(data.begin(), data.end(), [&](double& d) { d -= y_mean;}); // y-y_mean
      double var_x = 0.0;
      for(unsigned int i = 0; i < x.size(); i++) {
        var_x += x[i] * x[i];
      }
      double cov = 0.0;
      for(unsigned int i = 0; i < x.size(); i++) {
        cov += x[i] * data[i];
      }
      std::vector<double> result {cov/var_x, y_mean-cov/var_x * x_mean};
      return result;
    }
  }
}
