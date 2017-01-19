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
    class helper {
    private:
      std::vector<int> d_x_coord, d_y_coord;
      Matrixc d_data;
      int d_x_min, d_x_max, d_y_min, d_y_max;
      float d_prev_angle;

      inline float
      constrainAngle(float x){
        x = fmod(x + M_PI,2*M_PI);
        if (x < 0)
          x += 2*M_PI;
        return x - M_PI;
      }

      inline float
      angleConv(float angle){
        return fmod(constrainAngle(angle),2*M_PI);
      }

      inline float
      angleDiff(float a, float b){
        float dif = fmod(b - a + M_PI,2*M_PI);
        if (dif < 0)
          dif += 2*M_PI;
        return dif - M_PI;
      }

    public:
      helper(std::vector<int> y_coord);
      ~helper();
      gr_complex interp1d(gr_complex v1, gr_complex v2, int v2pos, int valpos);
      gr_complex get_value(int x, int y);
      void set_params(std::vector<int> x_coord, Matrixc data);
      std::vector<float> linear_regression(std::vector<float> data);
      void reset_angle();

      inline float
      unwrap(float newAngle){
        float new_angle = d_prev_angle - angleDiff(newAngle,angleConv(d_prev_angle));
        d_prev_angle = new_angle;
        return new_angle;
      }
    };
  }
}


#endif //GR_FBMC_INTERP2D_H
