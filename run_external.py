import subprocess

# This script continuously calls the reliability_external program to generate
# r values.  It then reads the most recent r-values and prints the r value of
# the first core.

def ms_to_hour(t):
    return t / (1000 * 60 * 60)

def hour_to_year(t):
    return t / (24 * 365)

def ms_to_year(t):
    return hour_to_year(ms_to_hour(t))

# cleanup
subprocess.run("rm -f rvalues.txt sums.txt", shell=True, check=True)

# delta_t = 100_000_000  # sample time in ms
# delta_t = 8640000000 # 100 days in ms
delta_t = 21600000000 # 250 days in ms

t = 0  # in ms
r_limit = 0.01
area = 0  # in hours

while True:
    subprocess.run("./reliability_external {} constant-temperature.log sums.txt rvalues.txt".format(delta_t), shell=True, check=True)
    t += delta_t
    # if t % 1_000_000 == 0:
    with open('rvalues.txt') as f:
        rvalue = float(f.read().split(' ')[0])
        print(ms_to_hour(t), rvalue)
        area += rvalue * ms_to_hour(delta_t)
        if rvalue <= r_limit:
            break;

print("R <= {} after {} years".format(r_limit, ms_to_year(t)))
print("Area under curve: {} years".format(hour_to_year(area)))
