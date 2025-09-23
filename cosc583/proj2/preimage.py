#!/usr/bin/env python3

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data = [
    pd.read_csv("preimage_8t_8b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_10b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_12b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_14b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_16b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_18b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_20b.csv").to_numpy().flatten(),
    pd.read_csv("preimage_8t_22b.csv").to_numpy().flatten(),
]

labels = [
    "8b",
    "10b",
    "12b",
    "14b",
    "16b",
    "18b",
    "20b",
    "22b",
]

plt.figure()
plt.violinplot(data, showmedians=True)
plt.yscale("log", base=2)

plt.plot([1, 8], [(2**8) / 2, (2**22) / 2])

plt.xlabel("Bit Legnth")
plt.ylabel("Attempts")
plt.title("Attempts for a preimage attack on SHA-1 truncated to bit length")

plt.xticks(np.arange(1, len(labels) + 1), labels)

plt.show()
