//
// Created by senpo on 17.01.17.
//

#ifndef GR_FBMC_PHASE_HELPER_H
#define GR_FBMC_PHASE_HELPER_H

#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
  namespace fbmc {
    class phase_helper {
    private:
      double d_prev_angle;

      inline double
      constrainAngle(double x){
        x = fmod(x + M_PI,2*M_PI);
        if (x < 0)
          x += 2*M_PI;
        return x - M_PI;
      }

      inline double
      angleConv(double angle){
        return fmod(constrainAngle(angle),2*M_PI);
      }

      inline double
      angleDiff(double a, double b){
        double dif = fmod(b - a + M_PI, 2 * M_PI);
        if (dif < 0)
          dif += 2*M_PI;
        return dif - M_PI;
      }

    public:
      phase_helper();
      ~phase_helper();
      std::vector<double> linear_regression(std::vector<double> data);
      void reset_angle();

      inline double
      unwrap(double newAngle){
        double new_angle = d_prev_angle - angleDiff(newAngle, angleConv(d_prev_angle));
        d_prev_angle = new_angle;
        return new_angle;
      }
    };
  }
}


#endif //GR_FBMC_PHASE_HELPER_H
