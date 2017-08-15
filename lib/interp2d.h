//
// Created by senpo on 17.01.17.
//

#ifndef GR_FBMC_INTERP2D_H
#define GR_FBMC_INTERP2D_H

#include <gnuradio/io_signature.h>
#include <cmath>
#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline.h>


namespace gr {
  namespace fbmc {
    class interp2d {
    private:
      const gsl_interp2d_type *d_T = gsl_interp2d_bilinear; // liniear interpolation
      std::vector<gr_complex> d_result;
      gsl_interp_accel *acc1;
      gsl_interp_accel *acc2;
      gsl_interp_accel *acc3;
      gsl_interp_accel *acc4;

    public:
      interp2d(int vec_len);
      ~interp2d();
      int interpolate(gr_complex*& out, int spanx, int spany, std::vector<std::vector<gr_complex> >& pilots);
      std::vector<gr_complex> interp1d(std::vector<int>& pilot_carriers, int span, std::vector<gr_complex>& symbol);
    };
  }
}


#endif //GR_FBMC_INTERP2D_H