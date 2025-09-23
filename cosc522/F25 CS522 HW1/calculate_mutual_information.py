#!/usr/bin/env python3

import pandas as pd
import math


def binary_entropy(ones, total):
    if total == 0 or ones == 0:
        return 0
    if ones == total:
        return 0

    return -(
        (ones / total) * math.log2(ones / total)
        + ((total - ones) / total) * math.log2((total - ones) / total)
    )


def mi_all_columns(data):
    total = len(data)
    ones = len(data[data[data.columns[-1]] == 1])
    entropy = binary_entropy(ones, total)

    mutual_information = {}

    for col in data.columns[:-1]:
        positive = data[data[col] == 1]
        negative = data[data[col] == 0]

        positive_ones = len(positive[positive[positive.columns[-1]] == 1])
        positive_total = len(positive)
        positive_entropy = binary_entropy(positive_ones, positive_total)
        positive_mi = (positive_total / total) * positive_entropy

        negative_ones = len(negative[negative[negative.columns[-1]] == 1])
        negative_total = len(negative)
        negative_entropy = binary_entropy(negative_ones, negative_total)
        negative_mi = (negative_total / total) * negative_entropy

        mi = entropy - (positive_mi + negative_mi)
        # print(f"Mutual information for {col}: {mi}")
        mutual_information[col] = mi

    return mutual_information


def best_split(mutual_information):
    max_label = ""
    max_val = 0

    for key, val in mutual_information.items():
        if val > max_val:
            max_label = key
            max_val = val

    return (max_label, max_val)


def split(data, split_col):
    left_tree = data[data[split_col] == 0]
    right_tree = data[data[split_col] == 1]
    left_tree = left_tree.drop(split_col, axis=1)
    right_tree = right_tree.drop(split_col, axis=1)

    return (left_tree, right_tree)


def main():
    data = pd.read_csv("data.txt")

    # first split
    mutual_information = mi_all_columns(data)
    split_col, split_val = best_split(mutual_information)
    print(f"First split on {split_col} @ {split_val}")
    print(f"Rows with 0 for {split_col} go left")
    print(f"Rows with 1 for {split_col} go right")
    left_tree, right_tree = split(data, split_col)

    print("")

    # left split
    mutual_information = mi_all_columns(left_tree)
    split_col, split_val = best_split(mutual_information)
    print(f"Left split on {split_col} @ {split_val}")
    left_left_tree, left_right_tree = split(left_tree, split_col)
    left_left_mode = left_left_tree[left_left_tree.columns[-1]].mode()[0]
    left_right_mode = left_right_tree[left_right_tree.columns[-1]].mode()[0]
    print(f"Rows with 0 for {split_col} majority vote goes to: {left_left_mode}")
    print(f"Rows with 1 for {split_col} majority vote goes to: {left_right_mode}")

    print("")

    # right split
    mutual_information = mi_all_columns(right_tree)
    split_col, split_val = best_split(mutual_information)
    print(f"Right split on {split_col} @ {split_val}")
    right_left_tree, right_right_tree = split(right_tree, split_col)
    right_left_mode = right_left_tree[right_left_tree.columns[-1]].mode()[0]
    right_right_mode = right_right_tree[right_right_tree.columns[-1]].mode()[0]
    print(f"Rows with 0 for {split_col} majority vote goes to: {right_left_mode}")
    print(f"Rows with 1 for {split_col} majority vote goes to: {right_right_mode}")


main()
