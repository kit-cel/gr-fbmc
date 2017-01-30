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
      phase_helper();
      ~phase_helper();
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


#endif //GR_FBMC_PHASE_HELPER_H
