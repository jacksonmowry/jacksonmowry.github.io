#!/usr/bin/env python3

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math

data = [
    pd.read_csv("collision_8t_8b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_10b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_12b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_14b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_16b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_18b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_20b.csv").to_numpy().flatten(),
    pd.read_csv("collision_8t_22b.csv").to_numpy().flatten(),
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


def val(bits):
    return math.sqrt((2 * 2**bits) * math.log(1 / (1 - 0.5)))


plt.figure()
plt.violinplot(data, showmedians=True)
plt.yscale("log", base=2)
plt.plot(
    [1, 2, 3, 4, 5, 6, 7, 8],
    [val(8), val(10), val(12), val(14), val(16), val(18), val(20), val(22)],
)

plt.xlabel("Bit Legnth")
plt.ylabel("Attempts")
plt.title("Attempts for a collision attack on SHA-1 truncated to bit length")

plt.xticks(np.arange(1, len(labels) + 1), labels)

plt.show()
