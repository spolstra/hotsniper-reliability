#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd
import sys

if len(sys.argv) != 2:
    print('usage: {} r-value csv file'.format(sys.argv[0]))
    sys.exit(1)

rvalue_file = sys.argv[1]

# Load your CSV file into a pandas dataframe
df = pd.read_csv(rvalue_file, index_col=0)

# Show the plot with all (sub)components
df.plot()
plt.show()

