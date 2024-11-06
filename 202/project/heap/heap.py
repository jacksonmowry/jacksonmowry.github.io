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


class Heap(MovingCameraScene):
    def construct(self):
        tree = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        tree[4].move_to(ORIGIN + UP * 2)
        tree[3].move_to(ORIGIN + LEFT * 2)
        tree[5].move_to(ORIGIN + RIGHT * 2)
        tree[1].move_to(ORIGIN + DOWN * 2 + LEFT * 3.5)
        tree[2].move_to(ORIGIN + DOWN * 2 + LEFT * 0.5)
        tree[6].move_to(ORIGIN + DOWN * 2 + RIGHT * 3.5)

        edges = [
            add_edge(tree[4], tree[3]),
            add_edge(tree[4], tree[5]),
            add_edge(tree[3], tree[1]),
            add_edge(tree[3], tree[2]),
            add_edge(tree[5], tree[6]),
        ]

        tree_group = VGroup(*edges, *list(tree.values()))

        self.camera.frame.set(width=self.camera.frame.get_width() * 1.75)
        self.play(Write(tree_group))
        counter = Text("0 Steps")
        counter.to_edge(UP + LEFT)
        self.add(counter)
        self.wait(4)

        go_away = VGroup(edges[0], edges[1], edges[-1], tree[4], tree[5], tree[6])
        self.bring_to_back(edges[0])
        self.bring_to_front(tree[3])
        edges[0].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(tree[3].get_center() + DOWN).set(
                    width=self.camera.frame.get_width() * 0.5
                ),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("1 Step")
        another_counter.scale(0.75)
        another_counter.next_to(tree[3], LEFT + UP)
        self.play(Write(another_counter))
        self.wait(2)

        go_away = VGroup(edges[2], edges[3], tree[2], tree[3])
        self.bring_to_back(edges[2])
        self.bring_to_front(tree[1])
        edges[2].set_opacity(0)

        self.play(
            AnimationGroup(
                self.camera.frame.animate.move_to(tree[1].get_center()).set(
                    width=self.camera.frame.get_width() * 0.5
                ),
                go_away.animate.set_opacity(0),
            )
        )

        another_counter = Text("2 Steps")
        another_counter.scale(0.5)
        another_counter.next_to(tree[1], LEFT + UP)
        self.play(Write(another_counter))
        self.wait(2)

        self.play(tree[1].animate.set_opacity(0))

        self.wait(2)

        equation = MathTex("\\text{2 Steps} = \\lfloor{}log_{2}(6)\\rfloor{}")
        equation.move_to(self.camera.frame.get_center())
        self.play(Transform(another_counter, equation))
        self.wait(8)
