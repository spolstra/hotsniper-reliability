#include <algorithm>
// #include <filesystem> // Not available in C++11
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <tuple>
#include <limits>

#include "rmodel.h"
#include "em_model.h"

using namespace std;

// TODO: How do we make sure that we are not working on stale partial sum
//       data. Data that is left over from a previous run? I don't see an
//       easy way to check this from this program.

// TODO: Abstract all the EM model specific stuff as much as possible.

/* Calculate the current R value for all cores.
 * Inputs:
 * - The delta t since the last measurement in milliseconds.
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

/* Conversion functions */
constexpr long double ms_to_hour(long double t) { return t / (60 * 60 * 1000); }

constexpr long double ns_to_hour(long double t) {
    return t / (static_cast<long double>(60L * 60L * 1000000000L));
}

constexpr long double hour_to_year(long double t) { return t / (24 * 365); }

/* Check if file 'name' exists.
 * Return true if it does, false otherwise.
 * Cannot use filesystem::exists() because need to use c++11. */
inline bool file_exists(const string &name) {
    ifstream f(name);
    return f.good();
}

/* Read header and temperatures.
 * Return a vector with the header names and a vector with the temperatures. */
pair<vector<string>, vector<long double>> read_instantaneous_temps(string inst_temp_filename) {
    /* Open temperatures file. */
    ifstream temperatures(inst_temp_filename);
    if (!temperatures) {
        throw runtime_error("Cannot open the instantaneous temperature file: " + inst_temp_filename);
    }

    /* Read header. */
    vector<string> header;
    string header_line;
    string component_name;
    getline(temperatures, header_line);
    stringstream ss = stringstream(header_line);
    while (ss >> component_name) {
        header.push_back(component_name);
    }

    /* Read core temperatures. */
    vector<long double> core_temperatures;
    string temperature;
    while (temperatures >> temperature) {
        core_temperatures.push_back(stold(temperature));
    }

    return make_pair(header, core_temperatures);
}


/* Read current sums from 'sum_filename'.
 * The format is a single line with the sums separated by whitespace:
 * sum_0 sum_1 ... sum_n
 *
 * Returns: a vector with the current sums of cores C0..Cn.  */
vector<long double> read_current_sums(string sum_filename,
                                      string r_values_filename,
                                      size_t num_temperatures) {
    vector<long double> current_sums;

    /* Open current sums file and perfom some sanity checking. */
    if (!file_exists(sum_filename)) {
        // If sums file does not exist return a vector with zeros.
        if (file_exists(r_values_filename)) {
            throw runtime_error("R values file exists but sums file does not!");
        }
        current_sums.insert(current_sums.begin(), num_temperatures, 0.0);
        return current_sums;
    }

    ifstream current_sums_stream(sum_filename);
    if (!current_sums_stream) {
        throw runtime_error("Error opening the current sum file: " +
                            sum_filename);
    }

    /* Finally read the current sum data from file. */
    string line;
    getline(current_sums_stream, line);
    stringstream strline(line);
    string current_sum;
    while (strline >> current_sum) {
        current_sums.push_back(stold(current_sum));
    }

    if (current_sums.size() != num_temperatures) {
        throw runtime_error(
            "The number of temperature values does not match the number of "
            "current sums");
    }

    return current_sums;
}

/* Write latest sums to 'sum_filename'. */
void write_current_sums(const vector<shared_ptr<Rmodel>> r_models, string sum_filename) {
    ofstream sum_file(sum_filename);

    bool first = true;
    for (const shared_ptr<Rmodel> &r_model : r_models) {
        if (first) first = false; else sum_file << "\t";
        sum_file << r_model->get_sum();
    }
    sum_file << endl;

}

/* Write the latest r-values to 'r_value_filename'. */
void write_r_values(const vector<shared_ptr<Rmodel>> &r_models,
        const vector<string> &header,string r_values_filename) {
    ofstream r_values_file(r_values_filename);

    /* Write header */
    bool first = true;
    for (const string &comp : header) {
        if (first) first = false; else r_values_file << "\t";
        r_values_file << comp;
    }
    r_values_file << endl;

    /* Write rvalues */
    first = true;

    r_values_file.precision(std::numeric_limits<long double>::max_digits10);
    for (const shared_ptr<Rmodel> &r_model : r_models) {
        if (first) first = false; else r_values_file << "\t";
        r_values_file << r_model->get_R();
    }
    r_values_file << endl;
}

/* Dump temperature measurements. */
void print_vector(vector<long double> data) {
    for (const auto &d : data) {
        cout << d << " ";
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    /* Handle command line arguments. */
    if (argc < 5 || argc > 6) {
        cerr << "Usage: " << argv[0]
             << " <delta_t (ms)> <hotspot_file> <current_sums_file> <r-values> [<acceleration_factor>]" << endl;
        return 1;
    }
    char *temperature_filename = argv[2];
    char *sum_filename = argv[3];
    char *r_values_filename = argv[4];
    long double delta_t = ms_to_hour(stold(argv[1]));  // Delta t in hours.
    if (argc == 6) {
        // Eg. speed up aging : 1000 * 60 * 60 * 24 * 100 = 8640_000_000
        //                      |---- one day ----|
        // Age with a delta of 100 days instead of 1ms
        delta_t *= stoll(argv[5]);
    }

    /* Read temperature and current sums from file. */
    vector<long double> temperatures;
    vector<string> header;
    tie(header, temperatures) = read_instantaneous_temps(temperature_filename);
    vector<long double> current_sums =
        read_current_sums(sum_filename, r_values_filename, temperatures.size());

    /* Create reliability models for all cores and initialize them with the
     * corresponding current sum. */
    vector<shared_ptr<Rmodel>> r_models;
    for (long double s : current_sums) {
        r_models.push_back(make_shared<EM_model>(s));
    }

    /* Update rmodels of cores with the latest temperature measurement. */
    for (auto it = r_models.begin(); it != r_models.end(); ++it) {
        (*it)->update(delta_t, temperatures[distance(r_models.begin(), it)]);
    }

    /*  Write back the updated current_sums of the rmodels. */
    write_current_sums(r_models, sum_filename);

    /* Write new r values to file. */
    write_r_values(r_models, header, r_values_filename);

    return 0;
}
