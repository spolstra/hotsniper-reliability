#include <cmath>

/*
    References:
        [1] Moghaddasi (2018) Instruction-level NBTI Stress Estimation and its
            Application in Runtime Aging Prediction for Embedded Processors
        [2] Chen (2014) Characterizing the Activity Factor in NBTI Aging Models
            for Embedded Cores
        [3] Kleeberger (2013) A compact model for NBTI degradation and recovery
            under use-profile variations and its application to aging analysis
            of digital integrated circuits
        [4] https://www.intel.com/content/dam/www/public/us/en/documents/presentation/revolutionary-22nm-transistor-technology-presentation.pdf
 */

class NBTI_model : public Rmodel {
    using Rmodel::Rmodel;

   public:
    void update_timestamp(long double new_time_stamp, long double temperature, long double voltage, long double stress) {
        long double delta_t = new_time_stamp - current_time_stamp;
        current_time_stamp = new_time_stamp;
        update(delta_t, temperature, voltage, stress);
    }

    void update(long double delta_t, long double temperature, long double voltage, long double stress) {
        if (delta_t < 0) {
            throw std::runtime_error("Negative delta_t is not allowed");
        }

        if (stress <= 0) {
            throw std::runtime_error("Negative stress is not allowed");
        }

        long double current_adf = adf(temperature, voltage, stress);

        current_delta_v = delta_v(current_adf, delta_t);
        current_state += delta_t / mttf();
        current_R = expl(-1 * powl(current_state, BETA));
        area_under_curve += current_R * delta_t;
    }

   private:
    long double adf(long double temp, long double voltage, long double stress) {
        long double temp_kelvin = celsius_to_kelvin(temp);
        // equation (7) from [1]
        return CONST_K * expl(-1 * CONST_E0 / (BOLTZMANNCONSTANT * temp_kelvin)) *
            expl((CONST_B * voltage) / (CONST_OX * BOLTZMANNCONSTANT * temp_kelvin)) *
            powl(stress, CONST_N);
    }

    long double delta_v(long double current_adf, long double delta_t) {
        // equation (11) from [1]
        return current_adf * powl(powl(current_delta_v / current_adf, 1/CONST_N) + delta_t, CONST_N);
    }

    long double mttf(void) {
        // equation (12) from [1]
        long double eadf = current_delta_v/powl(current_time_stamp, CONST_N);

        // equation (13) from [1]
        return powl((0.1 * CONST_INIT_V) / eadf, 1/CONST_N);
    }

    long double celsius_to_kelvin(long double temp) { return temp + ZERO_CEL_IN_KELVIN; }

    // NBTI related parameters
    const long double BOLTZMANNCONSTANT = 8.6173324 * 0.00001;
    const long double BETA              = 2;      // weibull scaling parameter
    const long double CONST_E0          = 0.1897; // from [1] (fitting parameter)
    const long double CONST_B           = 0.075;  // from [1] (fitting parameter)
    const long double CONST_N           = 0.167;  // from [2] (between 1/4 and 1/6 depending on diffusing species)
    const long double CONST_K           = 0.98;   // determined experimentally (fitting parameter)
    const long double CONST_OX          = 1.4;    // from [3] (effective oxide thickness in nm)
    const long double CONST_INIT_V      = 0.35;   // from [4] (initial threshold voltage)

    // Thermal model parameters
    const long double ZERO_CEL_IN_KELVIN = 273.15;
};
