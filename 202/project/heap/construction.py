#!/usr/bin/env python3

#!/usr/bin/env python3

from manim import *


def create_node(number: int):
    circ = Circle(fill_color=BLACK, fill_opacity=1)
    circ_text = Text(f"{number}")

    circ_text.move_to(circ.get_center())

    group = VGroup(circ, circ_text)

    return group


def create_square(number: int):
    circ = Square(fill_color=BLACK, fill_opacity=1)
    circ_text = Text(f"{number}")
    circ_text.scale(1.5)

    circ_text.move_to(circ.get_center())

    group = VGroup(circ, circ_text)

    return group


def add_edge(origin, dest):
    return Line(origin.get_center(), dest.get_center())


class Heap(MovingCameraScene):
    def construct(self):
        max_heap = {
            1: create_node(1),
            2: create_node(2),
            3: create_node(3),
            4: create_node(4),
            5: create_node(5),
            6: create_node(6),
            9: create_node(9),
        }

        max_heap[9].move_to(ORIGIN + UP * 2)
        max_heap[3].move_to(ORIGIN + LEFT * 3)
        max_heap[4].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 1.5)
        max_heap[1].move_to(ORIGIN + DOWN * 2.5 + LEFT * 4.5)
        max_heap[2].move_to(ORIGIN + DOWN * 2.5 + LEFT * 1.5)
        max_heap[5].move_to(ORIGIN + RIGHT * 3)
        max_heap[6].move_to(ORIGIN + DOWN * 2.5 + RIGHT * 4.5)

        max_edges = [
            add_edge(max_heap[9], max_heap[3]),
            add_edge(max_heap[9], max_heap[5]),
            add_edge(max_heap[3], max_heap[2]),
            add_edge(max_heap[3], max_heap[1]),
            add_edge(max_heap[5], max_heap[4]),
            add_edge(max_heap[5], max_heap[6]),
        ]

        max_group = VGroup(*max_edges, *list(max_heap.values()))
        max_group.move_to(ORIGIN)
        max_group.scale(0.5)

        self.play(Write(max_group))
        self.wait(0.5)

        self.play(max_group.animate.move_to(ORIGIN + LEFT * 3))
        self.wait(1)

        numbers = [
            create_square(9),
            create_square(3),
            create_square(5),
            create_square(1),
            create_square(2),
            create_square(4),
            create_square(6),
        ]

        numbers[0].move_to(ORIGIN + RIGHT)
        numbers[0].scale(0.3)
        self.play(Indicate(max_heap[9]))
        self.play(Write(numbers[0]))
        self.wait(0.15)

        numbers[1].move_to(ORIGIN + RIGHT * 1.7)
        numbers[1].scale(0.3)
        self.play(Indicate(max_heap[3]))
        self.play(Write(numbers[1]))
        self.wait(0.15)

        numbers[2].move_to(ORIGIN + RIGHT * 2.4)
        numbers[2].scale(0.3)
        self.play(Indicate(max_heap[5]))
        self.play(Write(numbers[2]))
        self.wait(0.15)

        numbers[3].move_to(ORIGIN + RIGHT * 3.1)
        numbers[3].scale(0.3)
        self.play(Indicate(max_heap[1]))
        self.play(Write(numbers[3]))
        self.wait(0.15)

        numbers[4].move_to(ORIGIN + RIGHT * 3.8)
        numbers[4].scale(0.3)
        self.play(Indicate(max_heap[2]))
        self.play(Write(numbers[4]))
        self.wait(0.15)

        numbers[5].move_to(ORIGIN + RIGHT * 4.5)
        numbers[5].scale(0.3)
        self.play(Indicate(max_heap[4]))
        self.play(Write(numbers[5]))
        self.wait(0.15)

        numbers[6].move_to(ORIGIN + RIGHT * 5.2)
        numbers[6].scale(0.3)
        self.play(Indicate(max_heap[6]))
        self.play(Write(numbers[6]))
        self.wait(0.15)

        a = CurvedArrow(numbers[0].get_top(), numbers[1].get_top(), radius=-0.5)
        b = CurvedArrow(numbers[0].get_top(), numbers[2].get_top(), radius=-1)
        self.play(Write(VGroup(a, b)))

        # add_edge(max_heap[9], max_heap[3]),
        # add_edge(max_heap[9], max_heap[5]),
        # add_edge(max_heap[3], max_heap[2]),
        # add_edge(max_heap[3], max_heap[1]),
        # add_edge(max_heap[5], max_heap[4]),
        # add_edge(max_heap[5], max_heap[6]),

        max_edges[0].set_z_index(0)
        max_heap[9].set_z_index(3)
        max_heap[3].set_z_index(3)
        self.play(
            AnimationGroup(
                max_edges[0].animate.set_color(BLUE),
                a.animate.set_color(BLUE),
            )
        )
        self.wait(0.25)

        max_edges[1].set_z_index(0)
        max_heap[9].set_z_index(3)
        max_heap[5].set_z_index(3)
        self.play(
            AnimationGroup(
                max_edges[1].animate.set_color(YELLOW),
                b.animate.set_color(YELLOW),
            )
        )

        c = CurvedArrow(numbers[1].get_bottom(), numbers[3].get_bottom(), color=PURPLE)
        d = CurvedArrow(numbers[1].get_bottom(), numbers[4].get_bottom(), color=ORANGE)
        max_heap[1].set_z_index(3)
        max_heap[2].set_z_index(3)

        self.play(
            AnimationGroup(
                Write(c),
                Write(d),
                max_edges[3].animate.set_color(PURPLE),
                max_edges[2].animate.set_color(ORANGE),
            )
        )

        e = CurvedArrow(
            numbers[2].get_top(), numbers[5].get_top(), color=PURE_GREEN, radius=-1.5
        )
        f = CurvedArrow(
            numbers[2].get_top(), numbers[6].get_top(), color=PURE_BLUE, radius=-2
        )
        max_heap[4].set_z_index(3)
        max_heap[6].set_z_index(3)

        self.play(
            AnimationGroup(
                Write(e),
                Write(f),
                max_edges[4].animate.set_color(PURE_GREEN),
                max_edges[5].animate.set_color(PURE_BLUE),
            )
        )

        self.wait(1)

        self.play(AnimationGroup(Indicate(max_heap[4]), Indicate(numbers[5])))
        self.wait(1)

        self.play(
            self.camera.frame.animate.move_to(numbers[3].get_center()).set(width=6)
        )

        self.wait(12)

        # Two reasons we use an array
        # 1. The complete binary tree property, meaning no holes will ever appear
        # 2. Implicit structure for child "Pointers"
        # It may not seem obvious, but let's add the array indicies and try to find a pattern

        indicies = [
            Text("0"),
            Text("1"),
            Text("2"),
            Text("3"),
            Text("4"),
            Text("5"),
            Text("6"),
        ]

        indicies[0].scale(0.25)
        indicies[0].move_to(numbers[0].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[1].scale(0.25)
        indicies[1].move_to(numbers[1].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[2].scale(0.25)
        indicies[2].move_to(numbers[2].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[3].scale(0.25)
        indicies[3].move_to(numbers[3].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[4].scale(0.25)
        indicies[4].move_to(numbers[4].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[5].scale(0.25)
        indicies[5].move_to(numbers[5].get_center() + UP * 0.2 + LEFT * 0.2)

        indicies[6].scale(0.25)
        indicies[6].move_to(numbers[6].get_center() + UP * 0.2 + LEFT * 0.2)

        self.play(Write(VGroup(*indicies)))
        self.wait(8)

        for n in max_heap.values():
            self.remove(n)
        for e in max_edges:
            self.remove(e)

        self.play(
            self.camera.frame.animate.move_to(numbers[3].get_center() + LEFT).set(
                width=8
            )
        )
        self.wait(4)

        obs = VGroup(
            MathTex("0 \\rightarrow{} 1, 2"),
            MathTex("1 \\rightarrow{} 3, 4"),
            MathTex("2 \\rightarrow{} 5, 6"),
        ).arrange(DOWN, buff=0.5)

        obs.move_to(numbers[0].get_center() + LEFT * 2)

        self.play(Write(obs))
        self.wait(10)

        formula = VGroup(
            MathTex("n \\rightarrow{}"), MathTex("(n*2)+1,"), MathTex("(n*2)+2")
        ).arrange(DOWN, buff=0.5)
        formula.move_to(numbers[0].get_center() + LEFT * 1.75)
        formula.scale(0.85)

        self.play(Transform(obs, formula))
        # This formula allows us to eliminate the need for pointers in our "tree" structure,
        # saving 16 bytes for each node, and a lot of time traversing pointers
        self.wait(10)

        self.remove(formula)
        self.remove(obs)

        # The formula to find your parent is even easier,
        # Just take your index subtract 1, and divide by 2
        formula = MathTex("\\text{parent} = \\\\(n-1)/2")
        formula.move_to(numbers[0].get_center() + LEFT * 1.75)
        self.play(Write(formula))
        self.wait(10)
