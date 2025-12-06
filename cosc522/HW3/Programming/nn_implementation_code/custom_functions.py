import torch

from torch.autograd import Function


class IdentityFunction(Function):
    """
    We can implement our own custom autograd Functions by subclassing
    torch.autograd.Function and implementing the forward and backward passes
    which operate on Tensors.
    """

    @staticmethod
    def forward(x):
        """
        In the forward pass we receive a Tensor containing the input and return
        a Tensor containing the output. ctx is a context object that can be used
        to stash information for backward computation. You can cache arbitrary
        objects for use in the backward pass using the ctx.save_for_backward method.
        """
        return x

    @staticmethod
    def backward(grad_output):
        """
        In the backward pass we receive a Tensor containing the gradient of the loss
        with respect to the output, and we need to compute the gradient of the loss
        with respect to the input.
        """
        return grad_output


class SigmoidFunction(Function):
    @staticmethod
    def forward(ctx, input):
        ctx.save_for_backward(input)

        return 1 / (1 + torch.exp(-input))

    @staticmethod
    def backward(ctx, grad_output):
        (input,) = ctx.saved_tensors

        sigmoid = 1 / (1 + torch.exp(-input))
        return grad_output * sigmoid * (1 - sigmoid)


class LinearFunction(Function):
    @staticmethod
    def forward(ctx, inp, weight, bias):
        ctx.save_for_backward(inp, weight, bias)

        return inp @ weight.T + bias

    @staticmethod
    def backward(ctx, grad_output):
        inp, weight, bias = ctx.saved_tensors

        grad_input = grad_output @ weight
        grad_weight = grad_output.T @ inp
        grad_bias = grad_output.sum(dim=0)
        return grad_input, grad_weight, grad_bias


class CrossEntropyFunction(Function):
    @staticmethod
    def forward(ctx, logits, target):

        activations = torch.nn.functional.log_softmax(logits, dim=1)

        loss = -activations.gather(1, target.view(-1, 1))
        loss = loss.mean()

        ctx.save_for_backward(logits, target)

        return loss

    @staticmethod
    def backward(ctx, grad_output):
        logits, target = ctx.saved_tensors

        activations = torch.nn.functional.softmax(logits, dim=1)
        ret = activations.clone()
        ret[range(logits.size(0)), target] -= 1
        ret *= grad_output / logits.size(0)

        return ret, None


if __name__ == "__main__":
    from torch.autograd import gradcheck

    num = 4
    inp = 3

    x = torch.rand((num, inp), requires_grad=True).double()

    sigmoid = SigmoidFunction.apply

    assert gradcheck(sigmoid, x)
    print("Backward pass for sigmoid function is implemented correctly")

    out = 2

    x = torch.rand((num, inp), requires_grad=True).double()
    weight = torch.rand((out, inp), requires_grad=True).double()
    bias = torch.rand(out, requires_grad=True).double()

    linear = LinearFunction.apply
    assert gradcheck(linear, (x, weight, bias))
    print("Backward pass for linear function is implemented correctly")

    activations = torch.rand((15, 10), requires_grad=True).double()
    target = torch.randint(10, (15,))
    crossentropy = CrossEntropyFunction.apply
    assert gradcheck(crossentropy, (activations, target))
    print("Backward pass for crossentropy function is implemented correctly")
