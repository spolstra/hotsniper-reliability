#include <cmath>

#include "wearout_model.h"

class EM_model : public Wearout_model {
   public:
    EM_model(void){};
    ~EM_model(void){};

    /* Wearout scale function for the EM fault model.
     *  temp in celsius. */
    long double operator()(long double temp) override {
        long double temp_kelvin = celsius_to_kelvin(temp);
        return (CONST_A0 * pow(CONST_JMJCRIT, -CONST_N) *
                exp(ACTIVATIONENERGY / (BOLTZMANCONSTANT * temp_kelvin))) /
               CONST_ERRF;
    }

   private:
    long double celsius_to_kelvin(long double temp) { return temp + ZERO_CEL_IN_KELVIN; }

    // Electro-Migration related parameters

    // const long double BETA = 2; // weibull scaling parameter (for CONST_ERRF)
    const long double ACTIVATIONENERGY = 0.48;
    // const long double ACTIVATIONENERGY = 0.8;  // From JEDEC page 5.
    const long double BOLTZMANCONSTANT = 8.6173324 * 0.00001;
    const long double CONST_JMJCRIT = 1500000;
    const long double CONST_N = 1.1;
    // const long double CONST_N = 2.0;  // From JEDEC page 5.
    const long double CONST_ERRF = 0.88623;  // math.gamma(1 + 1/BETA)
    const long double CONST_A0 = 30000;  // cross section = 1um^2  material constant = 3*10^13
    // Thermal model parameters
    const long double ZERO_CEL_IN_KELVIN = 273.15;
};
