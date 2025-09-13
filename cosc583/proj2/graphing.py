#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

data = [
    pd.read_csv('preimage_8t_8b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_10b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_12b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_14b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_16b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_18b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_20b.csv').to_numpy().flatten(),
    pd.read_csv('preimage_8t_22b.csv').to_numpy().flatten()
    ]

print(sorted(pd.read_csv('preimage_8t_22b.csv').to_numpy().flatten()))

# plt.yscale("log")
plt.violinplot(data)
plt.show()

data = [
    pd.read_csv('collision_8t_8b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_10b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_12b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_14b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_16b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_18b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_20b.csv').to_numpy().flatten(),
    pd.read_csv('collision_8t_22b.csv').to_numpy().flatten()
    ]

plt.violinplot(data)
plt.show()
