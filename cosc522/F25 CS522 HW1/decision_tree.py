#!/usr/bin/env python3
import pandas as pd
import math
import sys
import numpy as np


class Node:
    """
    Here is an arbitrary Node class that will form the basis of your decision
    tree.
    Note:
        - the attributes provided are not exhaustive: you may add and remove
        attributes as needed, and you may allow the Node to take in initial
        arguments as well
        - you may add any methods to the Node class if desired
    """

    def __init__(self):
        self._id = None
        self.left = None
        self.right = None
        self.split_attr = None
        self.leaf = None
        self.vote = None
        self.depth = None
        self.size = None
        self.data = None
        self.parent = None


id = 0


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


def train(data):
    return tree_recurse(data, 0)


def stopping_criteria(data, depth, split_val):
    global max_depth
    global min_mi
    global min_data_points

    if depth >= max_depth:
        return True

    if split_val <= min_mi:
        return True

    if len(data) < min_data_points:
        return True

    return False


def tree_recurse(data, depth):
    global id

    node = Node()
    node._id = id
    id = id + 1
    node.depth = depth
    node.size = len(data)
    node.data = data[:]
    mutual_information = mi_all_columns(data)
    split_col, split_val = best_split(mutual_information)

    if stopping_criteria(data, depth, split_val):
        node.leaf = True
        unique_arr, count_each = np.unique(
            data[data.columns[-1]].to_numpy(), return_counts=True
        )
        if len(count_each) == 2 and count_each[0] == count_each[1]:
            node.vote = 1
        else:
            node.vote = data[data.columns[-1]].mode()[0]
        return node
    else:
        left_tree, right_tree = split(data, split_col)
        node.split_attr = split_col
        node.leaf = False
        node.left = tree_recurse(left_tree, depth + 1)
        node.left.parent = node
        node.right = tree_recurse(right_tree, depth + 1)
        node.right.parent = node

    return node


def tree_print(node):
    for i in range(node.depth):
        print("| ", end="")
    if node.parent is not None:
        if node == node.parent.left:
            print(f"{node.parent.split_attr} = 0: ", end="")
        else:
            print(f"{node.parent.split_attr} = 1: ", end="")

    print(
        f"[{len(node.data[node.data[node.data.columns[-1]] == 0])} 0/{len(node.data[node.data[node.data.columns[-1]] == 1])} 1]"
    )

    if node.leaf is not True:
        if node.left is not None:
            tree_print(node.left)
        if node.right is not None:
            tree_print(node.right)


def inference(tree, sample):
    if tree.leaf is True:
        return tree.vote
    else:
        if sample[tree.split_attr] == 0:
            return inference(tree.left, sample)
        else:
            return inference(tree.right, sample)


scores = {}


def reduced_error_pruning(root, node, data):
    if node.leaf is True:
        return

    global scores
    node.leaf = True

    unique_arr, count_each = np.unique(
        data[data.columns[-1]].to_numpy(), return_counts=True
    )
    if len(count_each) == 2 and count_each[0] == count_each[1]:
        node.vote = 1
    else:
        node.vote = data[data.columns[-1]].mode()[0]

    correct = 0
    for _, row in data.iterrows():
        prediction = inference(root, row)
        correct += row.iloc[-1] == prediction

    loss = correct / len(data) * 100
    scores[node._id] = float(loss)

    node.leaf = False

    reduced_error_pruning(root, node.left, data)
    reduced_error_pruning(root, node.right, data)


def remove_split(node, idx):
    if node._id == idx:
        node.leaf = True
    else:
        if node.left is not None:
            remove_split(node.left, idx)
        if node.right is not None:
            remove_split(node.right, idx)


max_depth = 0
min_mi = 0
min_data_points = 0


def main():
    global scores
    global max_depth
    global min_mi
    global min_data_points
    training_data = pd.read_csv(sys.argv[1], sep="\t")
    validation_data = pd.read_csv(sys.argv[2], sep="\t")
    max_depth = int(sys.argv[3])
    min_mi = float(sys.argv[4])
    min_data_points = int(sys.argv[5])
    disable_pruning = True if sys.argv[6] == "True" else False

    node = train(training_data)
    tree_print(node)

    correct = 0

    for _, row in training_data.iterrows():
        prediction = inference(node, row)
        correct += row.iloc[-1] == prediction

    print(f"Min mi: {sys.argv[4]} Training: {correct/len(training_data)*100}%", end=" ")

    while True:
        correct = 0

        for _, row in validation_data.iterrows():
            prediction = inference(node, row)
            correct += row.iloc[-1] == prediction

        print(f"Validation: {round(correct/len(validation_data)*100, 4)}%")
        loss = correct / len(validation_data) * 100
        if disable_pruning is True:
            break

        reduced_error_pruning(node, node, validation_data)
        # print(scores)

        max_acc = 0
        max_idx = -1

        for k, v in scores.items():
            if v > max_acc:
                max_idx = k
                max_acc = v

        if max_acc >= loss:
            remove_split(node, max_idx)
            scores.clear()
        else:
            break

    tree_print(node)


main()

# if __name__ == "__main__":
#     pass
