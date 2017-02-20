//
// Created by senpo on 17.01.17.
//

#ifndef GR_FBMC_INTERP2D_H
#define GR_FBMC_INTERP2D_H

#include <gnuradio/io_signature.h>
#include <Eigen/Dense>
#include <cmath>
#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline.h>

typedef Eigen::Matrix<gr_complex, Eigen::Dynamic, Eigen::Dynamic> Matrixc;

namespace gr {
  namespace fbmc {
    class interp2d {
    private:
      const gsl_interp2d_type *d_T = gsl_interp2d_bilinear; // liniear interpolation

    public:
      interp2d();
      ~interp2d();
      Matrixc interpolate(int spanx, int spany, Matrixc pilots);
      Matrixc interp1d(std::vector<int> pilot_carriers, int span, Matrixc symbol);
    };
  }
}


#endif //GR_FBMC_INTERP2D_H
