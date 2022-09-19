#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

#include "reliability.h"

using namespace std;

/* Calculate the current R value for all cores.
 * Inputs:
 * - The delta t since the last measurement in nano seconds.
 * - Hotspot temperature file <hotspot_data> for the current core temperatures.
 * - File <current_sums> containing the current sums of dt/a(T) for all cores.
 *
 * Outputs:
 * - Writes the current R values for all cores to the <r_values> file.
 * - Writes the updated sums dt/a(T) for all cores to the <current_sums> file.
 *
 * Usage:
 * ./reliability_external <delta_t> <hotspot_data> <current_sums> <r_values>
 */


/* Read HotSniper temperature log file.
 * Returns a vector with all the core temperatures C0..Cn */
vector<long double> read_temps(string hotspot_file) {
    ifstream hotspot(hotspot_file);
    if (!hotspot) {
        throw runtime_error("Cannot open the hotspot file: " + hotspot_file);
    }

    regex core_pattern {R"(^C\d+)"};
    vector<long double> core_temperatures;
    string line;

    getline(hotspot, line);  // Check and then skip header
    if (line.find("Unit") == string::npos) {
        throw runtime_error("Missing header in temperature file");
    }

    while (getline(hotspot, line)) {
        smatch results;
        stringstream strline(line);
        string core_name;
        string core_temperature;

        strline >> core_name;
        if (!regex_search(core_name, results, core_pattern)) {
            break;  // This not a core temperature, stop.
        }

        strline >> core_temperature;
        core_temperatures.push_back(stold(core_temperature));
    }

    return core_temperatures;
}

/* Read current sums from current_sums_file.
 * The format is a single line with the sums separated by whitespace:
 * sum_0 sum_1 ... sum_n
 *
 * Returns: a vector with the current sums of cores C0..Cn.  */
vector<long double> read_current_sums(string current_sums_file) {
    vector<long double> current_sums;

    ifstream current_sums_stream(current_sums_file);
    if (!current_sums_stream) {
        throw runtime_error("Cannot open the current sum file: " + current_sums_file);
    }

    string line;
    getline(current_sums_stream, line);
    stringstream strline(line);

    string current_sum;
    while (strline >> current_sum) {
        current_sums.push_back(stold(current_sum));
    }

    return current_sums;
}

/* Dump temperature measurements. */
void print_vector(vector<long double> data) {
    for (const auto &d : data) {
        cout << d << " ";
    }
    cout << endl;
}

/* Conversion functions */
constexpr long double ms_to_hour(long double t) { return t / (60 * 60 * 1000); }

constexpr long double ns_to_hour(long double t) { return t / (static_cast<long double>(60L * 60L * 1000000000L)); }

constexpr long double hour_to_year(long double t) { return t / (24 * 365); }


int main(int argc, char *argv[]) {
    /* Handle command line inputs. */
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <delta_t> <hotspot_file> <current_sums_file>" << endl;
        return 1;
    }
    long double delta_t = ns_to_hour(stold(argv[1]));  // Delta t in hours.
    vector<long double> temperatures = read_temps(argv[2]);
    vector<long double> current_sums = read_current_sums(argv[3]);

    print_vector(temperatures);
    print_vector(current_sums);

    return 0;

    /* Calculate reliability numbers for core 0. */

 #if 0
    Rmodel rmodel(new EM_model());  // We use the EM failure model
    long double timestamp_h = 0;    // Current time in hours.
    const long double sample_rate = ms_to_hour(1);
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
            if (sample_count % 1000000 == 0) {
                cout << timestamp_h << ", " << R << endl;
            }
        }
    }

    cerr << "R <= " << R_limit << " after " << hour_to_year(timestamp_h) << endl;
    return 0;
#endif

}
