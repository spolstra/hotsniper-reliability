#include <cmath>
#include <iostream>

#include "em_model.h"

class Rmodel {
   public:
    /* We take ownership of alpha here. */
    Rmodel(Wearout_model *alpha)
        : current_R(1), current_time_stamp(0), current_sum(0),
        area_under_curve(0), alpha(alpha){};

    Rmodel(Wearout_model *alpha, long double sum)
        : current_R(1), current_time_stamp(0), current_sum(sum),
        area_under_curve(0), alpha(alpha){};

    Rmodel(const Rmodel &o)
        : current_R(o.current_R),
          current_time_stamp(o.current_time_stamp),
          current_sum(o.current_sum),
          alpha(o.alpha->clone()) { }

    ~Rmodel(void) {
        delete alpha; /* We own alpha, so we need to free it here. */
    };

    /* Calculate the new R value.
     * temp in celsius
     * new_time_stamp in ms

       We use equation (5) from the paper [1] (equation (11) in [2]):
       R(t) = exp(-pow((sum(T_j / alpha(T_j))), beta))

       [1] : "Lightweight and Open-source Framework for the Lifetime Estimation
       of Multicore Systems" 2014 by Cristiana Bolchini et al.
       [2] : "System-level reliability modeling for MPSoCs" by Yan Xiang.
       */

    /* Add a measurement using an absolute time stamp.
     * The delta_t is calculated using current time stamp. */
    long double add_measurement(long double temp, long double new_time_stamp) {
        long double delta_t = new_time_stamp - current_time_stamp;
        current_time_stamp = new_time_stamp;
        return add_measurement_delta(temp, delta_t);
    }

    /* Add a measurement with temperature temp over the period delta_t */
    long double add_measurement_delta(long double temp, long double delta_t) {
        if (delta_t < 0) {
            throw std::runtime_error("Negative delta_t is not allowed");
        }

        // Add measurement to the stored 'current_sum': sum(T_j / alpha(T_j))
        current_sum += delta_t / (*alpha)(temp);
        // std::cout << " current_sum: " << current_sum << std::endl;

        long double new_R = expl(-1 * powl(current_sum, BETA));
        if (new_R > current_R) {
            throw std::runtime_error("Reliability cannot increase over time");
        }
        area_under_curve += new_R * delta_t;
        current_R = new_R;
        return new_R;
    }

    long double get_R(void) const { return current_R; }
    long double get_sum(void) const { return current_sum; }
    long double get_area(void) const { return area_under_curve; }

   private:
    long double current_R;
    long double current_time_stamp;
    long double current_sum;
    long double area_under_curve;

    Wearout_model *alpha;

    const long double BETA = 2;  // weibull scaling parameter
};
