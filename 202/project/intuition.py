#!/usr/bin/env python3

from manim import *


class BinaryTree(MovingCameraScene):
    def construct(self):

        # Create an array of numbers
        numbers = [1, 3, 4, 5]
        array_elements = [Integer(num) for num in numbers]

        # Arrange the elements in a row
        array = VGroup(*array_elements).arrange(RIGHT, buff=0.5)

        # Add square brackets to represent the array
        left_bracket = Text("[")
        right_bracket = Text("]")

        # Position the brackets
        left_bracket.next_to(array, LEFT, buff=0.1)
        right_bracket.next_to(array, RIGHT, buff=0.1)

        # Group everything together
        full_array = VGroup(left_bracket, array, right_bracket)
        self.play(Write(full_array))
        self.play(self.camera.frame.animate.move_to(ORIGIN).set(width = 7))

        circ = Circle(fill_color = BLACK, fill_opacity=1)
        circ.move_to(array_elements[2].get_center())
        circ_text = Text("4")
        circ_text.move_to(circ.get_center())
        circ_group = VGroup(circ,circ_text)
        self.bring_to_front(circ_text)
        circ_group.scale(0.5)
        self.play(Transform(array_elements[2], circ))
        self.wait(1)

        g = VGroup(left_bracket, array_elements[0], array_elements[1])

        numbers = [1, 3]
        array_elements = [Integer(num) for num in numbers]

        # Arrange the elements in a row
        array = VGroup(*array_elements).arrange(RIGHT, buff=0.5)

        # Add square brackets to represent the array
        left_bracket = Text("[")
        right_bracket = Text("]")

        # Position the brackets
        left_bracket.next_to(array, LEFT, buff=0.1)
        right_bracket.next_to(array, RIGHT, buff=0.1)

        # Group everything together
        full_array = VGroup(left_bracket, array, right_bracket)
        full_array.move_to(array_elements[0].get_center()+LEFT+DOWN)
        self.play(Transform(g, full_array))
        self.play(self.camera.frame.animate.move_to(ORIGIN+DOWN+LEFT).set(width=5))
        self.wait(1)

        black_blob = Circle(fill_color = BLACK, fill_opacity=1, color = BLACK)
        black_blob.move_to(array_elements[0])

        circ = Circle(fill_color = BLACK, fill_opacity=1)
        circ.move_to(array_elements[1].get_center())
        circ_text = Text("3")
        circ_text.move_to(circ.get_center())
        circ_group_3 = VGroup(circ,circ_text)
        circ_group_3.scale(0.5)
        self.play(Write(black_blob))
        self.play(Transform(array_elements[1], circ_group_3))

        numbers = [1]
        array_elements = [Integer(num) for num in numbers]

        # Arrange the elements in a row
        array = VGroup(*array_elements).arrange(RIGHT, buff=0.5)

        # Add square brackets to represent the array
        left_bracket = Text("[")
        right_bracket = Text("]")

        # Position the brackets
        left_bracket.next_to(array, LEFT, buff=0.1)
        right_bracket.next_to(array, RIGHT, buff=0.1)

        # Group everything together
        full_array = VGroup(left_bracket, array, right_bracket)
        full_array.move_to(array_elements[0].get_center()+LEFT*2+DOWN*2)
        self.play(Write(full_array))
        t = Text("Base Case (size 1)")
        t.scale(0.25)
        t.next_to(full_array, DOWN*0.25)
        self.play(self.camera.frame.animate.move_to(ORIGIN+DOWN*2+LEFT*2).set(width=5))
        self.play(Write(t))
        self.wait(1)

        circ = Circle(fill_color = BLACK, fill_opacity=1)
        circ.move_to(full_array.get_center())
        circ_text = Text("1")
        circ_text.move_to(circ.get_center())
        circ_group = VGroup(circ,circ_text)
        circ_group.scale(0.5)

        self.play(t.animate.set_opacity(0))
        self.play(Transform(full_array, circ_group))
        self.wait(1)

        left_bracket = Text("[")
        left_bracket.next_to(circ, RIGHT*5)
        right_bracket = Text("]")
        right_bracket.next_to(left_bracket, RIGHT)
        g = VGroup(left_bracket, right_bracket)
        self.play(Write(g))
        self.play(self.camera.frame.animate.move_to(ORIGIN+DOWN*2+LEFT*1.5))

        t = Text("Base Case (size 0)")
        t.scale(0.25)
        t.next_to(g.get_center(), DOWN*1.5)
        self.play(Write(t))
        self.wait(1)

        self.play(t.animate.set_opacity(0))
        self.play(g.animate.set_opacity(0))
        self.wait(1)

        self.play(self.camera.frame.animate.move_to(circ_group_3.get_center()))
        self.play(self.camera.frame.animate.move_to(ORIGIN))

        black_blob = Circle(fill_color = BLACK, fill_opacity=1, color = BLACK)
        black_blob.scale(0.4)
        black_blob.move_to(ORIGIN+RIGHT*1.3)

        self.play(Write(black_blob))

        self.wait(1)

        numbers = [5]
        array_elements = [Integer(num) for num in numbers]

        # Arrange the elements in a row
        array = VGroup(*array_elements).arrange(RIGHT, buff=0.5)

        # Add square brackets to represent the array
        left_bracket = Text("[")
        right_bracket = Text("]")

        # Position the brackets
        left_bracket.next_to(array, LEFT, buff=0.1)
        right_bracket.next_to(array, RIGHT, buff=0.1)

        # Group everything together
        full_array = VGroup(left_bracket, array, right_bracket)
        full_array.move_to(ORIGIN + RIGHT*1.75 + DOWN)
        self.play(Write(full_array))
        self.wait(1)

        self.play(self.camera.frame.animate.move_to(full_array.get_center()))

        circ = Circle(fill_color = BLACK, fill_opacity=1)
        circ.move_to(full_array.get_center())
        circ_text = Text("5")
        circ_text.move_to(circ.get_center())
        circ_group = VGroup(circ,circ_text)
        circ_group.scale(0.5)

        self.play(Transform(full_array, circ_group))

        self.play(self.camera.frame.animate.move_to(ORIGIN).set(width = 15))
        self.wait(2)
