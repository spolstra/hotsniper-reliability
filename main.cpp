#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "rmodel.h"

using namespace std;

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
            cout << core_temp << "\t";
        }
        cout << endl;
    }
}

/* Convert milliseconds to hours. */
constexpr long double ms_to_hour(long double t) { return t / (60 * 60 * 1000); }
constexpr long double hour_to_year(long double t) { return t / (24 * 365); }

int main(void) {
    auto temps = read_temps();
    // print_temps(temps);

    /* Calculate reliability numbers for core 0. */

    Rmodel rmodel(new EM_model());  // We use the EM failure model
    long double timestamp_h = 0;    // Current time in hours.
    const long double sample_rate = ms_to_hour(100000);
    long double R = 1.0 ; // new processor reliability.
    long long sample_count = 0;

    cout << "time,R" << endl;
    const double long R_limit = 0.01;
    while (R > R_limit) {
        for (auto sample : temps) { // reuse of short temperature trace.
            timestamp_h += sample_rate;
            sample_count++;
            long double core0_temperature = sample[0];
            R = rmodel.add_measurement(core0_temperature, timestamp_h);
            if (sample_count % 100000 == 0) {
                cout << timestamp_h << ", " << R << endl;
            }
        }
    }

    cerr << "R <= " << R_limit << " after " << hour_to_year(timestamp_h) << " years" << endl;
    cerr << "Area under curve: " << hour_to_year(rmodel.get_area()) << " years" << endl;
    return 0;
}
