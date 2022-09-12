#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "em_model.h"

// TODO: check if we need to move all doubles to long double for extra
// precision.

using namespace std;

class Rmodel {
   public:
    /* We take ownership of alpha here. */
    Rmodel(Wearout_model *alpha)
        : current_R(1), current_time_stamp(0), current_sum(0), alpha(alpha){};

    ~Rmodel(void) {
        delete alpha; /* We own alpha, so we need to free it here. */
    };

    /* Calculate the new R value.
       We use equation (5) from the paper [1] (equation (11) in [2]):
       R(t) = exp(-pow((sum(T_j / alpha(T_j))), beta))

       [1] : "Lightweight and Open-source Framework for the Lifetime Estimation
       of Multicore Systems" 2014 by Cristiana Bolchini et al.
       [2] : "System-level reliability modeling for MPSoCs" by Yan Xiang.
       */
    long double add_measurement(long double temp, long double new_time_stamp) {
        long double delta_t = new_time_stamp - current_time_stamp;
        if (delta_t < 0) {
            throw runtime_error("Negative delta_t is not allowed");
        }

        // Add measurement to the stored 'current_sum': sum(T_j / alpha(T_j))
        current_sum += delta_t / (*alpha)(temp);

        long double new_R = exp(-1 * pow(current_sum, BETA));
        if (new_R > current_R) {
            throw runtime_error("Reliability cannot increase over time");
        }
        current_R = new_R;
        // std::cerr << "sum: " << current_sum << std::endl;
        return new_R;
    }

    long double get_R(void) { return current_R; }

   private:
    long double current_R;
    long double current_time_stamp;
    long double current_sum;
    Wearout_model *alpha;

    const long double BETA = 2;  // weibull scaling parameter
};

/* Read HotSniper temperature log file. */
vector<vector<long double>> read_temps(void) {
    vector<vector<long double>> temps;
    string line;
    getline(cin, line);  // skip header
    if (line.find("Core") == string::npos) {
        throw runtime_error("Missing header in temperature file");
    }

    while (getline(cin, line)) {
        vector<long double> sample;
        stringstream strline(line);
        string temperature;
        while (strline >> temperature) {
            sample.push_back(stold(temperature));
        }
        temps.push_back(sample);
    }
    return temps;
}

/* Dump temperature measurements. */
void print_temps(vector<vector<long double>> temps) {
    for (const auto &sample : temps) {
        for (auto core_temp : sample) {
            cout << core_temp << " ";
        }
        cout << endl;
    }
}

/* Convert milliseconds to hours. */
constexpr long double ms_to_hour(long double t) { return t / (60 * 60 * 1000); }

int main(void) {
    auto temps = read_temps();
    // print_temps(temps);

    /* Calculate reliability numbers for core 0. */

    Rmodel rmodel(new EM_model());  // We use the EM failure model
    long double timestamp_h = 0;    // Current time in hours.
    const long double sample_rate = ms_to_hour(1);  // sample rate is 1 ms.
    long double R = 1.0 ; // new processor reliability.
    long long sample_count = 0;
    while (R > 0.1) {
        for (auto sample : temps) { // reuse of short temperature trace.
            timestamp_h += sample_rate;
            sample_count++;
            long double core0_temperature = sample[0];
            R = rmodel.add_measurement(core0_temperature, timestamp_h);
            if (sample_count % 10000 == 0) {
                cout << "time: " << timestamp_h << " temp: " << core0_temperature << ", R: " << R << endl;
            }
        }
    }
    cout << "Processor failed after " << sample_count << " samples" << endl;
    cout << "Which is " << sample_count * sample_rate << " seconds" << endl;

    return 0;
}
