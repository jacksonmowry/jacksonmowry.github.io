import numpy as np
import seaborn as sns
import torch
import torchvision
import torchvision.transforms as transforms
from matplotlib import pyplot as plt
from tqdm import tqdm

import torch.nn as nn

from custom_modules import Linear, CrossEntropyLoss, Sigmoid


def batch_generator(tensor, batch_size):
    for start in range(0, tensor.size(0) - batch_size + 1, batch_size):
        yield tensor[start : start + batch_size]


class Network(nn.Module):
    def __init__(self, n_inputs, n_hidden1, n_output):
        super(Network, self).__init__()

        self.linear1 = Linear(n_inputs + 1, n_hidden1)
        self.linear2 = Linear(n_hidden1 + 1, n_output)

        self.sigmoid = Sigmoid()
        self.cel = CrossEntropyLoss()

    def forward(self, x):
        # First layer
        x = self.sigmoid(self.linear1(torch.cat((torch.ones(x.size(0), 1), x), dim=1)))
        # Second layer
        x = self.linear2(torch.cat((torch.ones((x.size(0), 1)), x), dim=1))
        return x


if __name__ == "__main__":
    trainset = torchvision.datasets.FashionMNIST(
        root="./data", train=True, download=True, transform=transforms.ToTensor()
    )

    testset = torchvision.datasets.FashionMNIST(
        root="./data", train=False, download=True, transform=transforms.ToTensor()
    )

    class_names = [
        "T-shirt",
        "Trouser",
        "Pullover",
        "Dress",
        "Coat",
        "Sandal",
        "Shirt",
        "Sneaker",
        "Bag",
        "Ankle boot",
    ]

    train_x = []
    train_y = []
    test_x = []
    test_y = []

    for image, label in trainset:
        train_x.append(image.view(-1))
        train_y.append(label)

    for image, label in testset:
        test_x.append(image.view(-1))
        test_y.append(label)

    train_x = torch.stack(train_x)
    train_y = torch.tensor(train_y)
    test_x = torch.stack(test_x)
    test_y = torch.tensor(test_y)

    hidden_size = 256

    model = Network(784, hidden_size, 10)
    learning_rate = 0.01
    epochs = 15
    batch_size = 1

    model.train()

    for epoch in range(epochs):
        training_loss = 0.0
        correct = 0
        total = 0

        for data, target in zip(
            batch_generator(train_x, batch_size), batch_generator(train_y, batch_size)
        ):
            output = model(data)

            loss = model.cel(output, target)

            loss.backward()

            # Update alpha/beta weight matrix
            with torch.no_grad():
                model.linear1.weight -= learning_rate * model.linear1.weight.grad
                model.linear1.bias -= learning_rate * model.linear1.bias.grad
                model.linear1.weight.grad.zero_()
                model.linear1.bias.grad.zero_()

                model.linear2.weight -= learning_rate * model.linear2.weight.grad
                model.linear2.bias -= learning_rate * model.linear2.bias.grad
                model.linear2.weight.grad.zero_()
                model.linear2.bias.grad.zero_()

            training_loss += loss.item() * batch_size

        # Validation set
        model.eval()
        test_output = model(test_x)
        test_loss = model.cel(test_output, test_y)
        _, predicted = torch.max(test_output.data, 1)
        total += test_y.size(0)
        correct += (predicted == test_y).sum().item()
        model.train()

        print(
            f"Epoch {epoch+1}/{epochs}, Training Loss: {training_loss/len(trainset):.4f}, Test Loss: {test_loss.item():.4f}, Test Accuracy: {100 * correct / total}%"
        )

    n_classes = 10

    model.eval()
    with torch.no_grad():
        train_outputs = model(train_x)
        test_outputs = model(test_x)

        _, train_predictions = torch.max(train_outputs, 1)
        _, test_predictions = torch.max(test_outputs, 1)

        train_cm = torch.zeros(n_classes, n_classes, dtype=torch.int64)
        test_cm = torch.zeros(n_classes, n_classes, dtype=torch.int64)

        for true, predicted in zip(train_y, train_predictions):
            train_cm[true.item(), predicted.item()] += 1

        for true, predicted in zip(test_y, test_predictions):
            test_cm[true.item(), predicted.item()] += 1

    # Find first mispredicted per class in test
    first_miss = {i: None for i in range(n_classes)}

    for i in range(len(test_x)):
        true_label = test_y[i].item()
        predicted_label = test_predictions[i].item()

        if true_label != predicted_label and first_miss[true_label] is None:
            first_miss[true_label] = (test_x[i], true_label, predicted_label)

    fig, axes = plt.subplots(2, 5, figsize=(15, 6))
    axes = axes.flatten()

    for i in range(10):
        ax = axes[i]
        ax.imshow(first_miss[i][0].reshape(28, 28).numpy(), cmap="gray")
        ax.set_title(
            f"True Class: {class_names[first_miss[i][1]]}, Predicted Class: {class_names[first_miss[i][2]]}",
            fontsize=10,
        )
        ax.axis("off")

    plt.tight_layout()
    plt.show()

    fig, axes = plt.subplots(1, 2, figsize=(12, 6))

    sns.heatmap(
        train_cm.numpy(),
        annot=True,
        fmt="d",
        cmap="Blues",
        ax=axes[0],
        xticklabels=class_names,
        yticklabels=class_names,
    )
    axes[0].set_title("Train Confusion Matrix")
    axes[0].set_xlabel("Predicted")
    axes[0].set_ylabel("True")

    sns.heatmap(
        test_cm.numpy(),
        annot=True,
        fmt="d",
        cmap="Blues",
        ax=axes[1],
        xticklabels=class_names,
        yticklabels=class_names,
    )
    axes[1].set_title("Test Confusion Matrix")
    axes[1].set_xlabel("Predicted")
    axes[1].set_ylabel("True")

    plt.tight_layout()
    plt.show()
