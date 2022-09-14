import numpy as np
import pandas as pd
from pandas import DataFrame
from scipy.constants import convert_temperature

import seaborn as sns
import matplotlib.pyplot as plt

# The same constants and parameters as in reliability.cpp and em_model.h
ACTIVATIONENERGY = 0.48
BOLTZMANCONSTANT = 8.6173324 * 0.00001
CONST_JMJCRIT = 1500000
CONST_N = 1.1
CONST_ERRF = 0.88623  # math.gamma(1 + 1/BETA)
CONST_A0 = 30000  # cross section = 1um^2  material constant = 3*10^13
BETA = 2

# equation (3)
# Black's equation (temp in celsius)
def em_model(temp):
    temp = convert_temperature(temp, 'Celsius', 'Kelvin')
    print(temp)
    return (CONST_A0 * np.power(CONST_JMJCRIT, -CONST_N) * np.exp(ACTIVATIONENERGY / (BOLTZMANCONSTANT * temp))) / CONST_ERRF

# equation (1)
def weibull(t, temp):
    return np.exp(-1 * np.power(t / (em_model(temp)), BETA))

# Calculate R for a time period of 5 days.
# R reaches ~0.001 after 5 days
ts = np.arange(0, 5 * 24 * 60 * 60, 3000)
rs = weibull(ts, 50)  # temperature fixed at 50 degress celsius
d = {"time": ts, "R": rs}
data = pd.DataFrame(d)
print(data)

# Plot R over time.
sns.set_theme(style="whitegrid")
fig, ax = plt.subplots()
ax.set_title('Plot of equation (1) and equation (5) from Caliper paper (isca04)')
sns.scatterplot(data=data, x="time", y="R", label='R equation(1)')

# Now read summation data from file.
sum_data = pd.read_csv("time-R.csv")
sns.scatterplot(data=sum_data, x="time", y="R", label='Summation equation(5)')

# Plot both in the same figure.
plt.legend(loc='best')
plt.show()
