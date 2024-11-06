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
        self.wait(1)

        t = Text("How do we know if this is a max heap?")
        t.to_edge(UP)
        self.play(Write(t))
        self.wait(4)

        # We can follow each pathway down and make sure nodes are ordered
        # > relative to their parent
        self.play(Indicate(max_heap[9]))
        self.play(Indicate(max_heap[3]))
        self.play(Indicate(max_heap[1]))
        self.wait(4)

        # Or said another way, we can go to each node and check if it is
        # less than its parent
        self.play(Indicate(max_heap[6]))
        self.play(Indicate(max_heap[5]), color=RED)
        self.wait(3)

        # If 6 and 5 swapped position we would have a max heap
        # We already know the operation to fix that, a simple swap
        self.play(Swap(max_heap[5], max_heap[6]))
        self.wait(3)

        # and now we can check if this is a max heap
        self.play(Indicate(max_heap[5]))
        self.play(Indicate(max_heap[6]))
        self.play(Indicate(max_heap[9]))
        self.wait(3)

        # If a tree needs no swapping then it is a max heap
        self.play(t.animate.set_opacity(0))
        for n in max_heap.values():
            self.remove(n)
        for e in max_edges:
            self.remove(e)
        self.wait(0.5)

        # Now we know how to push, pop, and organize a heap,
        # but how can we construct a heap?
        # We have 2 options, the first is more intuitive so we will
        # cover it first
        numbers = [9, 3, 1, 5, 2, 4, 7]
        array = [create_square(n) for n in numbers]
        array_objs = VGroup(*array).arrange(RIGHT, buff=0.2)
        array_objs.scale(0.5)
        self.play(Write(array_objs))
        self.wait(3)

        arrows = VGroup()

        for i in range(3):
            if i % 2 == 0:
                arrows.add(
                    CurvedArrow(
                        array[i].get_top(),
                        array[i * 2 + 1].get_top(),
                        radius=-1 * (array[i * 2 + 1].get_x() - array[i].get_x()),
                        color=random_color(),
                    ),
                )
                arrows.add(
                    CurvedArrow(
                        array[i].get_top(),
                        array[i * 2 + 2].get_top(),
                        radius=-1 * (array[i * 2 + 2].get_x() - array[i].get_x()),
                        color=random_color(),
                    ),
                )
            else:
                arrows.add(
                    CurvedArrow(
                        array[i].get_bottom(),
                        array[i * 2 + 1].get_bottom(),
                        color=random_color(),
                    ),
                )
                arrows.add(
                    CurvedArrow(
                        array[i].get_bottom(),
                        array[i * 2 + 2].get_bottom(),
                        color=random_color(),
                    ),
                )

        self.play(Write(arrows))
        self.wait(1)

        self.play(self.camera.frame.animate.move_to(ORIGIN + RIGHT * 4).set(width=20))
        self.wait(3)

        self.play(Indicate(array[0]))

        # Let's build a min heap
        bad_tree = []
        bad_tree_edges = []
        bad_tree.append(
            create_node(numbers[0])
            .move_to(ORIGIN + RIGHT * 9 + UP * 2)
            .scale(0.5)
            .set_z_index(3)
        )
        self.play(Write(bad_tree[-1]))
        self.play(Indicate(bad_tree[-1]), color=GREEN)

        self.play(Indicate(array[1]))
        bad_tree.append(
            create_node(numbers[1])
            .move_to(ORIGIN + RIGHT * 7.5 + UP * 1)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[-2], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[-2], color=RED))
        self.play(
            AnimationGroup(Swap(bad_tree[-1], bad_tree[-2]), Swap(array[0], array[1]))
        )
        self.play(Indicate(bad_tree[-1]), color=GREEN)

        self.play(Indicate(array[2]))
        bad_tree.append(
            create_node(numbers[2])
            .move_to(ORIGIN + RIGHT * 10.5 + UP * 1)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[-2], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[-2], color=RED))
        self.play(
            AnimationGroup(Swap(bad_tree[-1], bad_tree[-2]), Swap(array[1], array[2]))
        )
        self.play(Indicate(bad_tree[-1]), color=GREEN)

        self.play(Indicate(array[3]))
        bad_tree.append(
            create_node(numbers[3])
            .move_to(ORIGIN + RIGHT * 6.75 + DOWN * 0.5)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[0], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[0]), color=RED)
        self.play(
            AnimationGroup(Swap(bad_tree[0], bad_tree[-1]), Swap(array[3], array[0]))
        )
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[2]), color=GREEN)

        self.play(Indicate(array[4]))
        bad_tree.append(
            create_node(numbers[4])
            .move_to(ORIGIN + RIGHT * 8.25 + DOWN * 0.5)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[-2], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[-2]), color=RED)
        self.play(
            AnimationGroup(Swap(bad_tree[-2], bad_tree[-1]), Swap(array[3], array[4]))
        )
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[2]), color=GREEN)

        self.play(Indicate(array[5]))
        bad_tree.append(
            create_node(numbers[5])
            .move_to(ORIGIN + RIGHT * 9.75 + DOWN * 0.5)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[1], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[1]), color=GREEN)

        self.play(Indicate(array[6]))
        bad_tree.append(
            create_node(numbers[6])
            .move_to(ORIGIN + RIGHT * 11.25 + DOWN * 0.5)
            .scale(0.5)
            .set_z_index(3)
        )
        bad_tree_edges.append(add_edge(bad_tree[1], bad_tree[-1]))
        self.play(Write(bad_tree[-1]))
        self.play(Write(bad_tree_edges[-1]))
        self.play(Indicate(bad_tree[-1]))
        self.play(Indicate(bad_tree[1]), color=GREEN)

        big_oh = MathTex("O(log_{2}n)").move_to(ORIGIN + RIGHT * 4 + UP * 2).scale(1.5)
        real = MathTex("O(nlog_{2}n)").move_to(ORIGIN + RIGHT * 4 + UP * 2).scale(1.5)

        self.play(Write(big_oh))
        self.wait(3)
        self.play(Transform(big_oh, real))
        self.wait(5)

        # Turns out constructing the heap 1 by 1 is is a nlog_{2}n operation
        # Each insert is log_{2}n, but we perform the operation n times
        # Can we do any better?
        self.play(
            AnimationGroup(
                real.animate.set_opacity(0),
                big_oh.animate.set_opacity(0),
                arrows.animate.set_opacity(0),
                *[a.animate.set_opacity(0) for a in array],
                *[n.animate.set_opacity(0) for n in bad_tree],
                *[e.animate.set_opacity(0) for e in bad_tree_edges],
            )
        )
        self.remove(real)
        self.remove(big_oh)
        self.remove(arrows)
        self.remove(*array)
        self.remove(*bad_tree)
        self.remove(*bad_tree_edges)
        self.wait(0.5)

        numbers = [9, 3, 1, 5, 2, 4, 7]
        array = [create_square(n) for n in numbers]
        array_objs = VGroup(*array).arrange(RIGHT, buff=0.2)
        array_objs.scale(0.5)
        self.play(Write(array_objs))
        self.wait(1)

        arrows = VGroup()

        for i in range(3):
            if i % 2 == 0:
                arrows.add(
                    CurvedArrow(
                        array[i].get_top(),
                        array[i * 2 + 1].get_top(),
                        radius=-1 * (array[i * 2 + 1].get_x() - array[i].get_x()),
                        color=random_color(),
                    ),
                )
                arrows.add(
                    CurvedArrow(
                        array[i].get_top(),
                        array[i * 2 + 2].get_top(),
                        radius=-1 * (array[i * 2 + 2].get_x() - array[i].get_x()),
                        color=random_color(),
                    ),
                )
            else:
                arrows.add(
                    CurvedArrow(
                        array[i].get_bottom(),
                        array[i * 2 + 1].get_bottom(),
                        color=random_color(),
                    ),
                )
                arrows.add(
                    CurvedArrow(
                        array[i].get_bottom(),
                        array[i * 2 + 2].get_bottom(),
                        color=random_color(),
                    ),
                )

        self.play(Write(arrows))
        self.wait(5)

        heap_nodes = [create_node(n) for n in numbers]
        heap_nodes[0].move_to(ORIGIN)
        heap_nodes[1].move_to(ORIGIN + LEFT * 3 + DOWN * 2)
        heap_nodes[2].move_to(ORIGIN + RIGHT * 3 + DOWN * 2)
        heap_nodes[3].move_to(ORIGIN + LEFT * 4.5 + DOWN * 5)
        heap_nodes[4].move_to(ORIGIN + LEFT * 1.5 + DOWN * 5)
        heap_nodes[5].move_to(ORIGIN + RIGHT * 1.5 + DOWN * 5)
        heap_nodes[6].move_to(ORIGIN + RIGHT * 4.5 + DOWN * 5)

        edges = VGroup()
        for i in range(3):
            edges.add(
                add_edge(heap_nodes[i], heap_nodes[i * 2 + 1]).set_color(
                    arrows[i * 2].get_color()
                ),
            )
            edges.add(
                add_edge(heap_nodes[i], heap_nodes[i * 2 + 2]).set_color(
                    arrows[i * 2 + 1].get_color()
                )
            )

        heap_g = VGroup(edges, *heap_nodes)
        heap_g.move_to(ORIGIN + RIGHT * 8)
        heap_g.scale(0.5)

        self.play(Write(heap_g))
        self.wait(8)

        # This time around we will start by directly building the heap from an array
        # of unsorted numbers
        # Remember that we only care about 2 specific properties
        # Completeness and relative ordering
        # Instead of attempting to "order" the entire tree at once, how about we
        # ensure that each node is in order, we can do this by starting at the bottom of the heap

        # 7
        self.play(
            AnimationGroup(
                Indicate(array[-1], color=GREEN),
                Indicate(heap_nodes[-1], color=GREEN),
            ),
        )
        self.wait(1)

        # 4
        self.play(
            AnimationGroup(
                Indicate(array[-2], color=GREEN),
                Indicate(heap_nodes[-2], color=GREEN),
            ),
        )
        self.wait(1)

        # 2
        self.play(
            AnimationGroup(
                Indicate(array[-3], color=GREEN),
                Indicate(heap_nodes[-3], color=GREEN),
            ),
        )
        self.wait(1)

        # 5
        self.play(
            AnimationGroup(
                Indicate(array[-4], color=GREEN),
                Indicate(heap_nodes[-4], color=GREEN),
            ),
        )
        self.wait(1)

        # 1
        self.play(
            AnimationGroup(
                Indicate(array[2]),
                Indicate(heap_nodes[2]),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[5], color=GREEN),
                Indicate(heap_nodes[5], color=GREEN),
                Indicate(array[6], color=GREEN),
                Indicate(heap_nodes[6], color=GREEN),
            ),
        )
        self.wait(1)

        # 3
        self.play(
            AnimationGroup(
                Indicate(array[1]),
                Indicate(heap_nodes[1]),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[3], color=GREEN),
                Indicate(heap_nodes[3], color=GREEN),
                Indicate(array[4], color=RED),
                Indicate(heap_nodes[4], color=RED),
            ),
        )
        self.wait(1)

        self.play(
            AnimationGroup(Swap(array[1], array[4]), Swap(heap_nodes[1], heap_nodes[4]))
        )

        self.play(
            AnimationGroup(
                Indicate(array[1], color=GREEN),
                Indicate(heap_nodes[1], color=GREEN),
            ),
        )
        self.wait(1)

        # 9
        self.play(
            AnimationGroup(
                Indicate(array[0]),
                Indicate(heap_nodes[0]),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[2], color=RED),
                Indicate(heap_nodes[2], color=RED),
                Indicate(array[-3], color=RED),
                Indicate(heap_nodes[-3], color=RED),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[2], color=RED),
                Indicate(heap_nodes[2], color=RED),
            ),
        )

        self.play(
            AnimationGroup(Swap(array[0], array[2]), Swap(heap_nodes[0], heap_nodes[2]))
        )
        self.wait(1)

        self.play(
            AnimationGroup(
                Indicate(array[0]),
                Indicate(heap_nodes[0]),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[-1], color=RED),
                Indicate(heap_nodes[-1], color=RED),
                Indicate(array[-2], color=RED),
                Indicate(heap_nodes[-2], color=RED),
            ),
        )

        self.play(
            AnimationGroup(
                Indicate(array[-2], color=RED),
                Indicate(heap_nodes[-2], color=RED),
            ),
        )

        self.play(
            AnimationGroup(
                Swap(array[0], array[-2]), Swap(heap_nodes[0], heap_nodes[-2])
            )
        )
        self.wait(1)

        self.play(
            AnimationGroup(
                Indicate(array[0], color=GREEN),
                Indicate(heap_nodes[0], color=GREEN),
            ),
        )

        self.wait(2)

        big_oh = (
            MathTex("O(nlog_{2}n)?").move_to(ORIGIN + RIGHT * 4 + UP * 2).scale(1.5)
        )

        self.play(Write(big_oh))
        self.wait(10)
        # You may ask yourself, well isn't this the same nlogn time complexity that we had before?
        # Reorganizing each node is logn, and we visited each node once
        # But is reorganizing each node always logn? Well no, it is only logn from the bottom of the tree
        # Recognize that each time we move up a level we're doing less work

        bottom = MathTex("4 \\times{} 1").move_to(
            heap_nodes[-1].get_center() + RIGHT * 2
        )
        mid = MathTex("2 \\times{} 2").move_to(heap_nodes[-2].get_center() + RIGHT * 2)
        top = MathTex("1 \\times{} 3").move_to(heap_nodes[2].get_center() + RIGHT * 2)

        self.play(Write(VGroup(bottom, mid, top)))
        self.wait(5)

        # numbers = [9, 3, 1, 5, 2, 4, 7]

        self.play(
            AnimationGroup(
                arrows.animate.set_opacity(0),
                big_oh.animate.set_opacity(0),
                edges.animate.set_opacity(0),
                *[a.animate.set_opacity(0) for a in array],
                *[n.animate.set_opacity(0) for n in heap_nodes],
                top.animate.move_to(ORIGIN + UP * 2),
                mid.animate.move_to(ORIGIN + UP),
                bottom.animate.move_to(ORIGIN),
                self.camera.frame.animate.move_to(ORIGIN).set(width=15),
            )
        )
        self.remove(big_oh)
        self.remove(arrows)
        self.remove(*array)
        self.remove(*heap_nodes)
        self.remove(edges)

        self.wait(1)

        ans = MathTex("11")
        ans.move_to(ORIGIN + DOWN)
        self.play(Write(ans))
        self.wait(5)

        self.play(self.camera.frame.animate.move_to(ORIGIN + RIGHT + 2))

        fourth_row = MathTex("8 \\times{} 1").move_to(ORIGIN + RIGHT * 4 + UP * 3)
        third_row = MathTex("4 \\times{} 2").move_to(ORIGIN + RIGHT * 4 + UP * 2)
        second_row = MathTex("2 \\times{} 3").move_to(ORIGIN + RIGHT * 4 + UP * 1)
        first_row = MathTex("1 \\times{} 4").move_to(ORIGIN + RIGHT * 4)
        big_ans = MathTex("26").move_to(ORIGIN + RIGHT * 4 + DOWN)

        self.play(Write(VGroup(first_row, second_row, third_row, fourth_row, big_ans)))
        self.wait(5)

        self.play(self.camera.frame.animate.move_to(ORIGIN + RIGHT + 4 + DOWN * 2))

        fifth_row = MathTex("16 \\times{} 1").move_to(ORIGIN + RIGHT * 8 + UP * 4)
        fourth_row = MathTex("8 \\times{} 2").move_to(ORIGIN + RIGHT * 8 + UP * 3)
        third_row = MathTex("4 \\times{} 3").move_to(ORIGIN + RIGHT * 8 + UP * 2)
        second_row = MathTex("2 \\times{} 4").move_to(ORIGIN + RIGHT * 8 + UP * 1)
        first_row = MathTex("1 \\times{} 5").move_to(ORIGIN + RIGHT * 8)
        big_ans = MathTex("57").move_to(ORIGIN + RIGHT * 8 + DOWN)

        self.play(
            Write(
                VGroup(first_row, second_row, third_row, fourth_row, fifth_row, big_ans)
            )
        )
        self.wait(10)

        # Ok, so the numbers are roughly doubling each time, leading me
        # to belive this operation has linear time complexity, but let's look
        # at a few more data points to confirm
