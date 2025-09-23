#!/usr/bin/env python3

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# data = [
# ]

# labels = [
#     "8b",
#     "10b",
#     "12b",
#     "14b",
#     "16b",
#     "18b",
#     "20b",
#     "22b",
# ]

plt.figure()
plt.xlim(0,8)
plt.ylim(0,100)

# Training
plt.plot([0,1,2,3,4,5,6,7,8], [51.0,78.5,78.5,86.0,87.5,91.5,92.0,93.0,93.0], label='Training')
plt.plot([0,1,2,3,4,5,6,7,8], [59.79,72.16,72.16,82.47,76.29,74.23,76.29,76.29,76.29], label='Validation')
plt.legend(loc='upper left')

plt.xlabel("Tree Depth")
plt.ylabel("Accuracy")
plt.title("Sweeping Max Tree Depth on Heart Datset")

plt.savefig('q4_graph.jpg')
