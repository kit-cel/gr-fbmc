//
// Created by senpo on 17.01.17.
//

#ifndef GR_FBMC_INTERP2D_H
#define GR_FBMC_INTERP2D_H

#include <gnuradio/io_signature.h>
#include <Eigen/Dense>
#include <cmath>

typedef Eigen::Matrix<gr_complex, Eigen::Dynamic, Eigen::Dynamic> Matrixc;

namespace gr {
  namespace fbmc {
    class interp2d {
    private:
      std::vector<int> d_x_coord, d_y_coord;
      Matrixc d_data;
      int d_x_min, d_x_max, d_y_min, d_y_max;

    public:
      interp2d(std::vector<int> y_coord);
      ~interp2d();
      gr_complex interp1d(gr_complex v1, gr_complex v2, int v2pos, int valpos);
      gr_complex interpolate(int x, int y);
      void set_params(std::vector<int> x_coord, Matrixc data);
    };
  }
}


#endif //GR_FBMC_INTERP2D_H
