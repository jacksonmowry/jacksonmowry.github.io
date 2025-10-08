#!/usr/bin/env python3

import numpy as np
import time

file = "./temperature.csv"


def fit_ols(d_len):
    data = np.loadtxt(file, delimiter=",", dtype=str)[1:]

    train = data[:105242]
    test = data[105242:]

    X_train = train[:, 1]
    X_test = test[:, 1]

    # We have to drop 1 row here because we cannot predict the element
    # at the end of y if we include the last element of y as a feature
    X_train = np.array(
        [X_train[i : i + d_len] for i in range(len(X_train) - (d_len - 1))]
    ).astype(float)[:-1]
    X_test = np.array(
        [X_test[i : i + d_len] for i in range(len(X_test) - (d_len - 1))]
    ).astype(float)[:-1]

    # Add the bias '1' term as the first column of the matrix
    X_train = np.insert(X_train, 0, 1, axis=1)
    X_test = np.insert(X_test, 0, 1, axis=1)

    y_train = train[d_len:, 1].astype(float)
    y_test = test[d_len:, 1].astype(float)

    print(f"Fitting OLS with D = {d_len}")
    training_start = time.time()
    # Training goes here
    w = np.dot(
        np.linalg.inv(np.transpose(X_train) @ X_train) @ np.transpose(X_train), y_train
    )
    training_end = time.time()
    training_time = training_end - training_start

    error = 0

    for x, y in zip(X_test, y_test):
        error += (y - (np.transpose(w) @ x)) ** 2

    error /= len(y_test)

    print(f"Weights we learned including bias: \n{w}")
    print(f"Time taken to fit model: {training_time} seconds")
    print(f"MSE on X_test: {error}\n\n")


# fit_ols(10)
# fit_ols(50)
# fit_ols(100)
# fit_ols(500)


def fit_sgd():
    d_len = 17520
    data = np.loadtxt(file, delimiter=",", dtype=str)[1:]

    train = data[:105242]
    test = data[105242:]

    X_train = train[:, 1].astype(float)
    X_test = test[:, 1].astype(float)

    y_train = train[d_len:, 1].astype(float)
    y_test = test[d_len:, 1].astype(float)

    w = np.zeros((d_len + 1,))
    w[1:] = 1 / d_len
    w[0] = 1

    training_start = time.time()
    # Training goes here
    for epoch in range(20):
        # Loop through all training examples (batch size 1?)
        print(f"Epoch: {epoch}")
        for i in range(len(X_train) - (d_len - 1) - 1):
            x = np.insert(X_train[i : i + d_len], 0, 1, axis=0)
            y = y_train[i]
            gradient = (x * (np.transpose(w) @ x - y))
            w -= (1e-10) * gradient

    training_end = time.time()
    training_time = training_end - training_start

    train_error = 0
    train_count = 0
    test_error = 0
    test_count = 0

    for i in range(len(X_train) - (d_len - 1) - 1):
        x = np.insert(X_train[i : i + d_len], 0, 1, axis=0)
        y = y_train[i]
        train_error += (y - (np.transpose(w) @ x)) ** 2
        train_count += 1

    for i in range(len(X_test) - (d_len - 1) - 1):
        x = np.insert(X_test[i : i + d_len], 0, 1, axis=0)
        y = y_test[i]
        test_error += (y - (np.transpose(w) @ x)) ** 2
        test_count += 1

    train_error /= train_count
    test_error /= test_count

    print(f"Weights we learned including bias: \n{w[0:10]}")
    print(f"Time taken to fit model: {training_time} seconds")
    print(f"MSE on X_train: {train_error}")
    print(f"MSE on X_test: {test_error}\n\n")
    np.savetxt("final_weights.csv", w, delimiter=",")


fit_sgd()
