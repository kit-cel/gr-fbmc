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
#include <iostream>
#include <cstring>

namespace gr {
  namespace fbmc {
    interp2d::interp2d(int vec_len)
    {
      d_result.resize(vec_len);
      acc1 = gsl_interp_accel_alloc ();
      acc2 = gsl_interp_accel_alloc ();
      acc3 = gsl_interp_accel_alloc ();
      acc4 = gsl_interp_accel_alloc ();
    }

    interp2d::~interp2d() {
      gsl_interp_accel_free (acc1);
      gsl_interp_accel_free (acc2);
      gsl_interp_accel_free (acc3);
      gsl_interp_accel_free (acc4);
    }

    std::vector<gr_complex>
    interp2d::interp1d(std::vector<int>& pilot_carriers, int span, std::vector<gr_complex>& symbol) {
      double x[pilot_carriers.size()];
      double y_real[symbol.size()],  y_imag[symbol.size()];
      for (int i = 0; i < pilot_carriers.size(); i++) {
        x[i] = static_cast<double>(pilot_carriers[i]);
      }
      for (int i = 0; i < symbol.size(); i++) {
        y_real[i] = symbol[i].real();
        y_imag[i] = symbol[i].imag();
      }

      gsl_interp_accel_reset(acc1);
      gsl_interp_accel_reset(acc3);

      gsl_spline *spline_real = gsl_spline_alloc (gsl_interp_linear, pilot_carriers.size());
      gsl_spline *spline_imag = gsl_spline_alloc (gsl_interp_linear, pilot_carriers.size());
      gsl_spline_init (spline_real, x, y_real, pilot_carriers.size());
      gsl_spline_init (spline_imag, x, y_imag, pilot_carriers.size());
      for (int n = 0; n < span; ++n) {
        if(n <= pilot_carriers.front()) {
          d_result[n] = symbol[0]; // extrapolation

        }
        else if(n >= pilot_carriers.back()) {
          d_result[n] = symbol[symbol.size()-1]; // extrapolation
        }
        else {
          d_result[n] = gr_complex(gsl_spline_eval(spline_real, n, acc1),
                                 gsl_spline_eval(spline_imag, n, acc3));
        }
      }
      gsl_spline_free (spline_real);
      gsl_spline_free (spline_imag);

      return d_result;
    }

    int
    interp2d::interpolate(gr_complex* out, int spanx, int spany, std::vector<std::vector<gr_complex> >& pilots) {

      d_result.clear();
      int counter = 0;

      // fill pilot coordinate arrays
      const double xa[] {0.0, static_cast<double>(spanx)};
      double ya[spany];
      for (int y = 0; y < spany; y++) {
        ya[y] = y;
      }

      const size_t nx = sizeof(xa) / sizeof(double); /* x grid points */
      const size_t ny = sizeof(ya) / sizeof(double); /* y grid points */
      double *za_real = (double*)malloc(nx * ny * sizeof(double));
      double *za_imag = (double*)malloc(nx * ny * sizeof(double));
      gsl_interp2d *spline_real = gsl_interp2d_alloc(d_T, nx, ny);
      gsl_interp2d *spline_imag = gsl_interp2d_alloc(d_T, nx, ny);

      /* set z grid values */
      for (int k = 0; k < pilots.size(); k++) {
        for (int n = 0; n < pilots[0].size(); n++) {
          gsl_interp2d_set(spline_real, za_real, k, n, pilots[k][n].real());
          gsl_interp2d_set(spline_imag, za_imag, k, n, pilots[k][n].imag());
        }
      }
      /* initialize interpolation */

      gsl_interp_accel_reset(acc1);
      gsl_interp_accel_reset(acc2);
      gsl_interp_accel_reset(acc3);
      gsl_interp_accel_reset(acc4);

      gsl_interp2d_init(spline_real, xa, ya, za_real, nx, ny);
      gsl_interp2d_init(spline_imag, xa, ya, za_imag, nx, ny);

      /* interpolate N values in x and y and print out grid for plotting */
      for (int k = 0; k < spanx-1; k++) {
        std::vector<gr_complex> temp(spany);
        for (int n = 0; n < spany; n++) {
          out[0] = gr_complex(gsl_interp2d_eval_extrap(spline_real, xa, ya, za_real, k+1, n, acc1, acc2),
                               gsl_interp2d_eval_extrap(spline_imag, xa, ya, za_imag, k+1, n, acc3, acc4));
          out++;
        }
        counter++;
      }
      // last symbol does not need to be interpolated
      memcpy(out, pilots[1].data(), sizeof(gr_complex) * pilots[1].size());
      counter++;
      gsl_interp2d_free(spline_real);
      gsl_interp2d_free(spline_imag);
      free(za_real);
      free(za_imag);

      return counter;
    }
  }
}