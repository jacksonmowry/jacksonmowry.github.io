#!/usr/bin/env python3

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt("splitting_size.csv", delimiter=",", dtype=float)


plt.figure()
plt.xlim(0, 200)
plt.ylim(0, 100)

# print(np.transpose(data)[0:2])
# exit(1)
# Training
plt.plot(*np.transpose(data)[0:2], label="Training")
plt.plot(*np.transpose(data)[0:3:2], label="Validation")
# plt.plot([0,1,2,3,4,5,6,7,8], [59.79,72.16,72.16,82.47,76.29,74.23,76.29,76.29,76.29], label='Validation')
plt.legend(loc="upper left")

plt.xlabel("Minimum Node Count Splitting Size")
plt.ylabel("Accuracy")
plt.title("Sweeping Node Splitting Minimum Size")

plt.savefig("q5_graph.jpg")
