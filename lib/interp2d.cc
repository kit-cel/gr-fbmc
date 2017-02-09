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


#include "interp2d.h"

namespace gr {
  namespace fbmc {
    interp2d::interp2d()
    {
    }

    interp2d::~interp2d() {}

    void
    interp2d::set_params(std::vector<int> x_coord, std::vector<int> y_coord, Matrixc data) {
      // x is only known during one work function, so it has to be reset every time
      d_x_coord = x_coord;
      d_y_coord = y_coord;
      d_data = data;
      if(d_x_coord.size() != data.cols()) {
        throw std::runtime_error("interp2d: Size of x coords has to be width of data matrix");
      }
      if(d_y_coord.size() != data.rows()) {
        throw std::runtime_error("interp2d: Size of y corrds has to be height of data matrix");
      }
      // find index borders in y direction
      d_y_min = *std::min_element(d_y_coord.begin(), d_y_coord.end());
      d_y_max = *std::max_element(d_y_coord.begin(), d_y_coord.end());
      d_x_min = *std::min_element(d_x_coord.begin(), d_x_coord.end());
      d_x_max = *std::max_element(d_x_coord.begin(), d_x_coord.end());
    }

    gr_complex
    interp2d::interp1d(gr_complex v1, gr_complex v2, int v2pos, int valpos) {
      // 1d linear interpolation between two points
      if(v2pos < valpos) {
        throw std::runtime_error("interp1d: requested value outside interpolation range");
      }
      gr_complex a = (v2 - v1)/gr_complex(v2pos, 0);
      return a*gr_complex(valpos, 0) + v1;
    }

    gr_complex
    interp2d::interpolate(int x, int y) {
      int xstart = -1; // -1 as extrapolation flag
      int ystart = -1;
      // find x base value leq to desired x
      for(unsigned int i = 0; i < d_x_coord.size(); i++) {
        if (d_x_coord[i] > x) {
          break;
        }
        xstart = i;
      }
      // find y base value leq to desired y
      for(unsigned int i = 0; i < d_y_coord.size(); i++) {
        if (d_y_coord[i] > y) {
          break;
        }
        ystart = i;
      }
      if(xstart == d_x_coord.size()-1 || xstart ==  -1) { // case: x extrapolation
        xstart = (xstart == -1) ? 0 : xstart;
        if (ystart == d_y_coord.size() - 1 || ystart == -1) { // case: y extrapolation
          ystart = (ystart == -1) ? 0 : ystart;
          return d_data(ystart, xstart); // simple extrapolation
        } else { // case: y interpolation
          return interp1d(d_data(ystart, xstart), // upper base value
                          d_data(ystart + 1, xstart), // lower base value
                          d_y_coord[ystart + 1] - d_y_coord[ystart], // x difference between two bases
                          y - d_y_coord[ystart]); // x difference between requested index and base
        }
      }
      else { // case: x interpolation
        if (ystart == d_y_coord.size() - 1 || ystart == -1) { // case: y extrapolation
          ystart = (ystart == -1) ? 0 : ystart;
          return interp1d(d_data(ystart, xstart), // left base value
                          d_data(ystart, xstart + 1), // right base value
                          d_x_coord[xstart + 1] - d_x_coord[xstart], // x difference between two bases
                          x - d_x_coord[xstart]); // x difference between requested index and base
        }
        else { // case: y interpolation
          gr_complex interpx1, interpx2;
          interpx1 = interp1d(d_data(ystart, xstart), // upper left base value
                              d_data(ystart, xstart + 1), // upper right base value
                              d_x_coord[xstart + 1] - d_x_coord[xstart], // x difference between two bases
                              x - d_x_coord[xstart]); // x difference between requested index and base

          interpx2 = interp1d(d_data(ystart + 1, xstart), // lower left base value
                              d_data(ystart + 1, xstart + 1), // lower right base value
                              d_x_coord[xstart + 1] - d_x_coord[xstart], // x difference between two bases
                              x - d_x_coord[xstart]); // x difference between requested index and base

          return interp1d(interpx1, // interpolated upper value for desired x
                          interpx2, // interpolated lower value for desired x
                          d_y_coord[ystart + 1] - d_y_coord[ystart], // y difference between two bases
                          y - d_y_coord[ystart]); // y difference between requested index and base
        }
      }
    }
  }
}
