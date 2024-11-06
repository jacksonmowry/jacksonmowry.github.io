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
        self.wait(4)

        self.play(
            AnimationGroup(
                tree[6].animate.set_opacity(0), edges[4].animate.set_opacity(0)
            )
        )
        self.wait(6)

        self.play(
            AnimationGroup(
                tree[5].animate.set_opacity(0), edges[1].animate.set_opacity(0)
            )
        )
        self.wait(4)

        g = VGroup(*edges)
        self.play(
            AnimationGroup(
                g.animate.set_opacity(0),
                tree[4].animate.move_to(ORIGIN + RIGHT * 2.5),
                tree[3].animate.move_to(ORIGIN + RIGHT + DOWN * 3),
                tree[2].animate.move_to(ORIGIN + UP * 3),
                tree[1].animate.move_to(ORIGIN + LEFT * 2.5),
            )
        )

        edges = VGroup(
            add_edge(tree[2], tree[1]),
            add_edge(tree[2], tree[4]),
            add_edge(tree[4], tree[3]),
        )
        self.bring_to_back(edges)
        self.bring_to_front(tree_group)

        self.play(Write(edges))
        self.play(Indicate(tree[4]))

        self.wait(1)

        self.play(self.camera.frame.animate.move_to(ORIGIN + RIGHT * 3))

        tex = MathTex("log_{2}n")
        tex.scale(3)
        tex.move_to(ORIGIN + RIGHT * 8)
        self.play(Write(tex))
        self.wait(1)

        self.play(
            AnimationGroup(
                tex.animate.set_opacity(0),
                tree_group.animate.set_opacity(0),
                edges.animate.set_opacity(0),
                self.camera.frame.animate.move_to(ORIGIN).set(width=10),
            )
        )
        self.wait(1)

        t = Text("How can we take\nadvantage of this?")
        t.move_to(ORIGIN)
        self.play(Write(t))
        self.wait(6)
        self.play(t.animate.set_opacity(0))

        first = Text("1. Every heap will be a complete binary tree")
        second = Text(
            "2. We will ignore strict ordering,\ninstead relying on relative ordering"
        )
        first.scale(0.75)
        second.scale(0.75)

        first.move_to(ORIGIN + UP)
        second.move_to(ORIGIN + DOWN)

        g = VGroup(first, second)

        self.play(Write(g))
        self.wait(12)

        self.play(g.animate.set_opacity(0))

        title = Text("What is a complete binary tree?")
        title.scale(0.5)
        title.move_to(ORIGIN + UP * 2)
        self.play(Write(title))

        # Valid
        tree = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        tree[4].move_to(ORIGIN + UP * 2)
        tree[3].move_to(ORIGIN + LEFT * 3)
        tree[5].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 1.5)
        tree[1].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        tree[2].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        tree[6].move_to(ORIGIN + RIGHT * 3)

        edges = [
            add_edge(tree[4], tree[3]),
            add_edge(tree[4], tree[6]),
            add_edge(tree[3], tree[1]),
            add_edge(tree[3], tree[2]),
            add_edge(tree[6], tree[5]),
        ]

        valid_group = VGroup(*edges, *list(tree.values()))

        valid_group.move_to(ORIGIN + LEFT * 3.5)
        valid_group.scale(0.3)

        # Last Row Failure
        tree = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        tree[4].move_to(ORIGIN + UP * 2)
        tree[3].move_to(ORIGIN + LEFT * 3)
        tree[5].move_to(ORIGIN + RIGHT * 3)
        tree[1].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        tree[2].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        tree[6].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 4.5)

        edges = [
            add_edge(tree[4], tree[3]),
            add_edge(tree[4], tree[5]),
            add_edge(tree[3], tree[1]),
            add_edge(tree[3], tree[2]),
            add_edge(tree[5], tree[6]),
        ]

        last_row_group = VGroup(*edges, *list(tree.values()))

        last_row_group.move_to(ORIGIN)
        last_row_group.scale(0.3)

        # Middle Row Failure
        tree = {
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        tree[4].move_to(ORIGIN + UP * 2)
        tree[5].move_to(ORIGIN + RIGHT * 3)
        tree[6].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 4.5)

        edges = [
            add_edge(tree[4], tree[5]),
            add_edge(tree[5], tree[6]),
        ]

        middle_row_group = VGroup(*edges, *list(tree.values()))

        middle_row_group.move_to(ORIGIN + RIGHT * 3.5)
        middle_row_group.scale(0.3)

        l1 = Line(
            start=ORIGIN + UP * 1.5 + LEFT * 1.85,
            end=ORIGIN + DOWN * 1.5 + LEFT * 1.85,
            stroke_width=0.85,
        )
        l2 = Line(
            start=ORIGIN + UP * 1.5 + RIGHT * 1.85,
            end=ORIGIN + DOWN * 1.5 + RIGHT * 1.85,
            stroke_width=0.85,
        )

        valid = Text("✔ Valid")
        valid.next_to(valid_group, DOWN)

        invalid = Text(" Invalid")
        cross = Cross()
        cross.scale(0.25)
        cross.next_to(invalid, LEFT)
        invalid_g = VGroup(cross, invalid)
        invalid_g.next_to(last_row_group, DOWN)

        invalid_2 = Text(" Invalid")
        cross_2 = Cross()
        cross_2.scale(0.25)
        cross_2.next_to(invalid_2, LEFT)
        invalid_g_2 = VGroup(cross_2, invalid_2)
        invalid_g_2.next_to(middle_row_group, DOWN)

        trees = VGroup(
            valid_group,
            valid,
            l1,
            last_row_group,
            invalid_g,
            l2,
            middle_row_group,
            invalid_g_2,
        )

        self.play(Write(trees))
        self.wait(20)

        self.play(
            AnimationGroup(trees.animate.set_opacity(0), title.animate.set_opacity(0))
        )

        heap = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        heap[1].move_to(ORIGIN + UP * 2)
        heap[3].move_to(ORIGIN + LEFT * 3)
        heap[6].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 1.5)
        heap[5].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        heap[4].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        heap[2].move_to(ORIGIN + RIGHT * 3)

        edges = [
            add_edge(heap[1], heap[3]),
            add_edge(heap[1], heap[2]),
            add_edge(heap[3], heap[5]),
            add_edge(heap[3], heap[4]),
            add_edge(heap[2], heap[6]),
        ]

        valid_group = VGroup(*edges, *list(heap.values()))
        valid_group.move_to(ORIGIN + LEFT * 0.25)
        valid_group.scale(0.5)

        title = Text("What is relative ordering?")
        title.scale(0.5)
        title.move_to(ORIGIN + UP * 2)
        self.play(Write(title))
        self.play(Write(valid_group))
        self.wait(8)

        self.play(Indicate(heap[1]))
        self.play(Indicate(heap[3]))
        self.play(Indicate(heap[5]))

        self.play(Indicate(heap[1]))
        self.play(Indicate(heap[3]))
        self.play(Indicate(heap[4]))

        self.play(Indicate(heap[1]))
        self.play(Indicate(heap[2]))
        self.play(Indicate(heap[6]))

        self.play(Swap(heap[3], heap[2]))

        self.wait(6)

        self.play(valid_group.animate.move_to(ORIGIN + LEFT * 3.5).scale(0.5))

        max_heap = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
        }

        max_heap[6].move_to(ORIGIN + UP * 2)
        max_heap[3].move_to(ORIGIN + LEFT * 3)
        max_heap[4].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 1.5)
        max_heap[1].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        max_heap[2].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        max_heap[5].move_to(ORIGIN + RIGHT * 3)

        max_edges = [
            add_edge(max_heap[6], max_heap[3]),
            add_edge(max_heap[6], max_heap[5]),
            add_edge(max_heap[3], max_heap[2]),
            add_edge(max_heap[3], max_heap[1]),
            add_edge(max_heap[5], max_heap[4]),
        ]

        max_group = VGroup(*max_edges, *list(max_heap.values()))
        max_group.move_to(ORIGIN + RIGHT * 1)
        max_group.scale(0.5)

        self.play(Write(max_group))
        self.wait(8)

        self.play(Indicate(max_heap[6]))
        self.play(Indicate(max_heap[3]))
        self.play(Indicate(max_heap[1]))

        self.play(
            AnimationGroup(
                valid_group.animate.move_to(ORIGIN + LEFT * 0.25).scale(2),
                max_group.animate.move_to(ORIGIN + LEFT * 0.25),
            )
        )
        self.wait(8)

        valid_group.animate.set_opacity(0)
        self.remove(valid_group)
        self.play(title.animate.set_opacity(0))
        pop = Text("Heap Pop & Percolate Down").move_to(title.get_center())
        self.remove(title)
        self.wait(5)

        # heap pop
        self.play(Write(pop))
        self.play(Swap(max_heap[6], max_heap[4]))
        self.wait(5)
        self.play(max_edges[4].animate.set_opacity(0))
        self.play(max_heap[6].animate.set_opacity(0))
        self.play(Indicate(max_heap[4]))
        self.play(AnimationGroup(Indicate(max_heap[3]), Indicate(max_heap[5])))
        self.wait(5)
        self.play(Indicate(max_heap[5], color=RED))
        self.wait(5)

        self.play(Swap(max_heap[4], max_heap[5]))
        self.play(Indicate(max_heap[4]))

        self.wait(6)

        # heap pop
        self.play(Swap(max_heap[5], max_heap[2]))
        self.wait(5)
        self.play(max_edges[2].animate.set_opacity(0))
        self.play(max_heap[5].animate.set_opacity(0))
        self.wait(5)

        self.play(Indicate(max_heap[2]))
        self.wait(1)
        self.play(AnimationGroup(Indicate(max_heap[3]), Indicate(max_heap[4])))
        self.wait(1)
        self.play(Indicate(max_heap[4], color=RED))

        self.play(Swap(max_heap[2], max_heap[4]))
        self.wait(6)

        self.remove(max_group)
        self.remove(max_heap[1])
        self.remove(max_heap[2])
        self.remove(max_heap[3])
        self.remove(max_heap[4])
        self.remove(max_heap[5])
        self.remove(max_heap[6])
        self.remove(max_edges[0])
        self.remove(max_edges[1])
        self.remove(max_edges[2])
        self.remove(max_edges[3])
        self.remove(max_edges[4])
        pop.set_opacity(0)

        push = Text("Heap Push & Percolate Up").move_to(pop.get_center())
        self.remove(pop)
        self.play(Write(push))

        max_heap = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
            9: create_node(9),
        }

        max_heap[6].move_to(ORIGIN + UP * 2)
        max_heap[3].move_to(ORIGIN + LEFT * 3)
        max_heap[4].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 1.5)
        max_heap[1].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        max_heap[2].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        max_heap[5].move_to(ORIGIN + RIGHT * 3)
        max_heap[9].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 4.5)

        max_edges = [
            add_edge(max_heap[6], max_heap[3]),
            add_edge(max_heap[6], max_heap[5]),
            add_edge(max_heap[3], max_heap[2]),
            add_edge(max_heap[3], max_heap[1]),
            add_edge(max_heap[5], max_heap[4]),
            add_edge(max_heap[5], max_heap[9]),
        ]

        max_heap[9].set_opacity(0)
        max_edges[5].set_opacity(0)

        max_group = VGroup(*max_edges, *list(max_heap.values()))
        max_group.move_to(ORIGIN)
        max_group.scale(0.5)

        self.play(Write(max_group))
        self.wait(6)

        self.bring_to_front(max_heap[5])
        self.bring_to_front(max_heap[9])
        self.bring_to_back(max_edges[5])
        self.bring_to_front(max_heap[5])
        self.bring_to_back(max_edges[5])
        self.play(
            AnimationGroup(
                max_edges[5].animate.set_opacity(1),
                max_heap[9].animate.set_opacity(1),
            )
        )
        self.bring_to_back(max_edges[5])
        self.bring_to_front(max_heap[5])

        self.wait(4)

        self.play(Indicate(max_heap[9]))
        self.play(Indicate(max_heap[5]))
        self.wait(2)

        self.play(Swap(max_heap[9], max_heap[5]))
        self.wait(2)
        self.play(Indicate(max_heap[9]))
        self.play(Indicate(max_heap[6]))
        self.wait(2)

        self.play(Swap(max_heap[9], max_heap[6]))

        self.wait(6)
