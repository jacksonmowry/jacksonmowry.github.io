#!/usr/bin/env python3

from manim import *
import math


def myfunc(x):
    levels = math.ceil(math.log2(x))
    sum = 0
    denom = 2
    for i in range(levels):
        sum += (i + 1) * (x / denom)
        denom *= 2

    return sum


class axes(Scene):
    def construct(self):
        axes = Axes(
            x_range=[0, 100, 10],
            y_range=[-1, 200, 10],
            tips=False,
            axis_config={"numbers_to_include": range(0, 201, 20)},
        )
        x_lab = axes.get_x_axis_label("X axis")
        y_lab = axes.get_y_axis_label("Y axis")

        self.play(Write(axes), Write(x_lab), Write(y_lab))

        log2 = axes.plot(
            myfunc,
            color=BLUE,
            x_range=[1, 100, 0.1],
            use_smoothing=False,
        )

        linear = axes.plot(
            lambda x: 2 * x,
            color=RED,
            x_range=[0, 100, 0.1],
        )

        log2_label = axes.get_graph_label(
            log2,
            "\\sum_{i=1}^{\\lceil{}log_{2}x\\rceil{}}\\frac{ix}{2^{i}}",
            x_val=90,
            direction=DOWN * 2,
        )

        linear_label = axes.get_graph_label(
            linear,
            "2x",
            x_val=80,
            direction=UP,
        )

        g = VGroup(log2, linear, log2_label, linear_label)

        self.play(Write(g))
        self.wait(12)
        # In the end we can see that constructing a heap from an array (or heapifying an array)
        # takes roughly 2n time, or without the constant O(n)

        self.clear()
        # We can now summarize what we've learned

        t0 = MathTable(
            [
                ["\\textbf{Operation}", "\\textbf{Time Complexity}"],
                ["\\text{Heap Pop}", "O(log_{2}n)"],
                ["\\text{Heap Push}", "O(log_{2}n"],
                ["\\text{Naive Constructor}", "O(nlog_{2}n)"],
                ["\\text{Heapify Array}", "O(n)"],
            ]
        ).move_to(ORIGIN)

        self.play(Write(t0))
        self.wait(12)
