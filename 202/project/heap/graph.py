from manim import *
import math


class axes(Scene):
    def construct(self):
        axes = Axes(
            x_range=(0, 32),
            y_range=(-1, 32),
            tips=False,
            axis_config={"numbers_to_include": range(0, 33, 2)},
        )
        x_lab = axes.get_x_axis_label("X axis")
        y_lab = axes.get_y_axis_label("Y axis")

        self.play(Write(axes), Write(x_lab), Write(y_lab))

        log2 = axes.plot(
            lambda x: math.trunc(math.log2(math.floor(x))),
            color=BLUE,
            x_range=[1, 32, 0.01],
            use_smoothing=False,
        )

        linear = axes.plot(
            lambda x: x,
            color=RED,
            x_range=[0, 32, 0.001],
        )

        log2_label = axes.get_graph_label(
            log2,
            "\\lfloor{}log_{2}(\\lfloor{}x\\rfloor{})\\rfloor{}",
            x_val=28,
            direction=UP,
        )

        linear_label = axes.get_graph_label(
            linear,
            "x",
            x_val=28,
            direction=DOWN,
        )

        g = VGroup(log2, linear, log2_label, linear_label)

        self.play(Write(g))
        self.wait(10)
