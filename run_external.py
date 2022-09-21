import subprocess

# This script continuously calls the reliability_external program to generate
# r values.  It then reads the most recent r-values and prints the r value of
# the first core.

def ms_to_hour(t):
    return t / (1000 * 60 * 60)

# cleanup
subprocess.run("rm -f rvalues.txt sums.txt", shell=True, check=True)

delta_t = 100_000_000  # sample time in ms
t = 0
while True:
    subprocess.run("./reliability_external {} hotspot-sample-output.txt sums.txt rvalues.txt".format(delta_t), shell=True, check=True)
    t += delta_t
    if t % 10_000_000 == 0:
        with open('rvalues.txt') as f:
            rvalues = f.read().split(' ')[0]
            print(ms_to_hour(t), rvalues)

