#!/usr/bin/env python3

from manim import *


def create_node(number: int):
    circ = Circle(fill_color=BLACK, fill_opacity=1)
    circ_text = Text(f"{number}")

    circ_text.move_to(circ.get_center())

    group = VGroup(circ, circ_text)

    return group


def add_edge(origin, dest):
    return Line(origin.get_center(), dest.get_center())


class LL(MovingCameraScene):
    def construct(self):
        tree = {
            6: create_node(6),
            5: create_node(5),
            4: create_node(4),
            3: create_node(3),
            2: create_node(2),
            1: create_node(1),
        }

        tree[6].move_to(ORIGIN + UP * 2)
        tree[5].move_to(ORIGIN + LEFT)
        tree[4].move_to(ORIGIN + DOWN * 2 + LEFT * 2)
        tree[3].move_to(ORIGIN + DOWN * 4 + LEFT * 3)
        tree[2].move_to(ORIGIN + DOWN * 6 + LEFT * 4)
        tree[1].move_to(ORIGIN + DOWN * 8 + LEFT * 5)

        edges = [
            add_edge(tree[6], tree[5]),
            add_edge(tree[5], tree[4]),
            add_edge(tree[4], tree[3]),
            add_edge(tree[3], tree[2]),
            add_edge(tree[2], tree[1]),
        ]

        tree_group = VGroup(*edges, *list(tree.values()))

        self.camera.frame.move_to(
            (tree[4].get_center() + tree[3].get_center()) / 2 + RIGHT
        )
        self.camera.frame.set(width=self.camera.frame.get_width() * 1.75)
        self.play(Write(tree_group))

        counter = Text("0 Steps")
        counter.to_edge(UP + LEFT)
        self.add(counter)
        self.wait(5)

        go_away = VGroup(tree[6], edges[0])
        edges[0].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(tree[3].get_center() + RIGHT).set(
                    width=self.camera.frame.get_width() * 0.80
                ),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("1 Step")
        another_counter.scale(0.75)
        another_counter.to_edge(LEFT)
        self.play(Write(another_counter))
        self.wait(2)

        go_away = VGroup(tree[5], edges[1])
        edges[1].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(
                    (tree[3].get_center() + tree[2].get_center()) / 2 + RIGHT
                ).set(width=self.camera.frame.get_width() * 0.78),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("2 Steps")
        another_counter.scale(0.65)
        another_counter.next_to(tree[4], LEFT, buff=2)
        self.play(Write(another_counter))
        self.wait(2)

        go_away = VGroup(tree[4], edges[2])
        edges[2].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to((tree[2].get_center())).set(
                    width=self.camera.frame.get_width() * 0.7
                ),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("3 Steps")
        another_counter.scale(0.60)
        another_counter.next_to(tree[3], LEFT, buff=1.8)
        self.play(Write(another_counter))
        self.wait(2)

        go_away = VGroup(tree[3], edges[3])
        edges[3].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(
                    (tree[2].get_center() + tree[1].get_center()) / 2 + LEFT
                ).set(width=self.camera.frame.get_width() * 0.7),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("4 Steps")
        another_counter.scale(0.55)
        another_counter.next_to(tree[2], LEFT, buff=1.7)
        self.play(Write(another_counter))
        self.wait(2)

        go_away = VGroup(tree[2], edges[4])
        edges[4].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(
                    (tree[1].get_center()) + LEFT * 1.5
                ).set(width=self.camera.frame.get_width() * 0.7),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("5 Steps")
        another_counter.scale(0.55)
        another_counter.next_to(tree[1], LEFT, buff=1.7)
        self.play(Write(another_counter))
        self.wait(2)

        self.play(tree[1].animate.set_opacity(0))

        self.wait(1)

        equation = MathTex("\\text{5 Steps} = \\text{Height}")
        equation.move_to(self.camera.frame.get_center())
        self.play(Transform(another_counter, equation))
        self.wait(2)
