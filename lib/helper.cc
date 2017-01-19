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


#include "helper.h"
#include <numeric>

namespace gr {
  namespace fbmc {
    helper::helper(std::vector<int> y_coord) :
    d_y_coord(y_coord)
    {
      // find index borders in y direction
      d_y_min = *std::min_element(d_y_coord.begin(), d_y_coord.end());
      d_y_max = *std::max_element(d_y_coord.begin(), d_y_coord.end());
      d_prev_angle = 0.0;
    }

    helper::~helper() {}

    void
    helper::reset_angle() {
      d_prev_angle = 0.0;
    }

    std::vector<float>
    helper::linear_regression(std::vector<float> data) {
      // return vector of {a, b} with linear regression line ax+b for y data points
      std::vector<float> x;
      for(unsigned int i = 0; i < data.size(); i++) {
        x.push_back(i);
      }
      float x_mean = (data.size()-1)/2.0;
      float y_mean = std::accumulate(data.begin(), data.end(), 0.0)/data.size();
      std::for_each(x.begin(), x.end(), [&](float& d) { d -= x_mean;}); // x-x_mean
      std::for_each(data.begin(), data.end(), [&](float& d) { d -= y_mean;}); // y-y_mean
      float var_x = 0.0;
      for(unsigned int i = 0; i < x.size(); i++) {
        var_x += x[i] * x[i];
      }
      float cov = 0.0;
      for(unsigned int i = 0; i < x.size(); i++) {
        cov += x[i] * data[i];
      }
      std::vector<float> result {cov/var_x, y_mean-cov/var_x * x_mean};
      return result;
    }

    void
    helper::set_params(std::vector<int> x_coord, Matrixc data) {
      // x is only known during one work function, so it has to be reset every time
      d_x_coord = x_coord;
      d_data = data;
      if(d_x_coord.size() != data.cols()) {
        throw std::runtime_error("interp2d: Size of x coords has to be width of data matrix");
      }
      if(d_y_coord.size() != data.rows()) {
        throw std::runtime_error("interp2d: Size of y corrds has to be height of data matrix");
      }
      d_x_min = *std::min_element(d_x_coord.begin(), d_x_coord.end());
      d_x_max = *std::max_element(d_x_coord.begin(), d_x_coord.end());
    }

    gr_complex
    helper::interp1d(gr_complex v1, gr_complex v2, int v2pos, int valpos) {
      // 1d linear interpolation between two points
      if(v2pos < valpos) {
        throw std::runtime_error("interp1d: requested value outside interpolation range");
      }
      gr_complex a = (v2 - v1)/gr_complex(v2pos, 0);
      return a*gr_complex(valpos, 0) + v1;
    }

    gr_complex
    helper::get_value(int x, int y) {
      gr_complex result;
      if(x > d_x_max || x < d_x_min || y < d_y_min || y > d_y_max) {
        // extrapolation range
        int x_extrap, y_extrap;
        if(x < d_x_min) {
          x_extrap = d_x_min;
        }
        else { x_extrap = d_x_max; }
        if( y < d_y_min ) {
          y_extrap = d_y_min;
        }
        else { y_extrap = d_y_max; }
        return d_data(y_extrap, x_extrap);
      }
      int xstart, ystart;
      gr_complex interpx1, interpx2;
      for(unsigned int i = 0; i < d_x_coord.size(); i++) {
        if (d_x_coord[i] <= x) {
          xstart = i;
        }
      }
      for(unsigned int i = 0; i < d_y_coord.size(); i++) {
        if (d_y_coord[i] <= y) {
          ystart = i;
        }
      }
      interpx1 = interp1d(d_data(ystart, xstart), // upper left base value
                          d_data(ystart, xstart+1), // upper right base value
                          d_x_coord[xstart+1] - d_x_coord[xstart], // x difference between two bases
                          x-d_x_coord[xstart]); // x difference between requested index and base

      interpx2 = interp1d(d_data(ystart+1, xstart), // lower left base value
                          d_data(ystart+1, xstart+1), // lower right base value
                          d_x_coord[xstart+1] - d_x_coord[xstart], // x difference between two bases
                          x-d_x_coord[xstart]); // x difference between requested index and base

      return interp1d(interpx1, // interpolated upper value for desired x
                      interpx2, // interpolated lower value for desired x
                      d_y_coord[ystart+1] - d_y_coord[ystart], // y difference between two bases
                      y-d_y_coord[ystart]); // y difference between requested index and base
    }
  }
}
