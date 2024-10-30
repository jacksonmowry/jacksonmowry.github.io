from manim import *


class BinaryTree(MovingCameraScene):
    def construct(self):
        keys = [1, 3, 4, 5, 6, 8, 9]
        one = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        three = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        four = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        five = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        six = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        eight = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle
        nine = Circle(fill_color=BLACK, fill_opacity=1)  # create a circle

        five.move_to(ORIGIN)

        three.next_to(five, LEFT*4+DOWN*2, buff=0.5)
        one.next_to(three, LEFT*0.01+DOWN*2, buff=0.5)
        four.next_to(three, RIGHT*0.01+DOWN*2, buff=0.5)

        eight.next_to(five, RIGHT*4+DOWN*2, buff=0.5)
        six.next_to(eight, LEFT*0.01+DOWN*2, buff=0.5)
        nine.next_to(eight, RIGHT*0.01+DOWN*2, buff=0.5)

        one_text = Text("1", font_size=72)
        one_text.move_to(one.get_center())

        three_text = Text("3", font_size=72)
        three_text.move_to(three.get_center())

        four_text = Text("4", font_size=72)
        four_text.move_to(four.get_center())

        five_text = Text("5", font_size=72)
        five_text.move_to(five.get_center())

        six_text = Text("6", font_size=72)
        six_text.move_to(six.get_center())

        eight_text = Text("8", font_size=72)
        eight_text.move_to(eight.get_center())

        nine_text = Text("9", font_size=72)
        nine_text.move_to(nine.get_center())

        lines = [Line(five.get_center(), three.get_center(), color=WHITE),
                 Line(three.get_center(), one.get_center()),
                 Line(three.get_center(), four.get_center()),
                 Line(five.get_center(), eight.get_center()),
                 Line(eight.get_center(), six.get_center()),
                 Line(eight.get_center(), nine.get_center())]


        group = VGroup(*lines, five, five_text, three, three_text, eight, eight_text, one, one_text, four, four_text, six, six_text, nine, nine_text)
        group.scale(0.5)
        group.move_to(ORIGIN)
        self.play(Write(group))
        self.wait(1)

        self.play(self.camera.frame.animate.move_to(three.get_center()+DOWN).set(width=three.width*7))
        self.wait(15)

        self.clear()

        # Create an array of numbers
        numbers = [1, 3, 4, 5, 6, 8, 9]
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

        # Display the array
        full_array.move_to(full_array.get_center() + UP+LEFT)
        self.play(self.camera.frame.animate.move_to(full_array.get_center()))
        self.play(Write(full_array))
        self.wait(2)

        arrow = Arrow(end = array_elements[3].get_center()+DOWN*0.1, start = array_elements[3].get_center()+DOWN*2)

        self.play(Write(arrow))
        self.wait(2)

        self.play(self.camera.frame.animate.move_to(ORIGIN+RIGHT*2).set(width=full_array.width*3))
        self.wait(2)

        building_tree = VGroup(five)
        building_tree.move_to(ORIGIN+RIGHT*7+UP*0.75)
        building_tree.scale(0.5)
        self.play(Write(building_tree))
        five_text.move_to(five.get_center())
        self.play(Transform(array_elements[3], five_text))
        self.bring_to_front(five_text)
        self.wait(2)

        self.play(arrow.animate.set_opacity(0))
        self.wait(2)

        left_numbers = [1,3,4]
        left_array = [Integer(num) for num in left_numbers]
        left_array_group = VGroup(*left_array).arrange(RIGHT, buff=0.5)
        left_array_group.move_to(array_elements[1].get_center() + DOWN*1 + LEFT*1)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(left_array_group,LEFT,buff=0.1)
        rb.next_to(left_array_group,RIGHT,buff=0.1)
        left_group = VGroup(lb, left_array_group, rb)

        right_numbers = [6,8,9]
        right_array = [Integer(num) for num in right_numbers]
        right_array_group = VGroup(*right_array).arrange(RIGHT, buff=0.5)
        right_array_group.move_to(array_elements[5].get_center() + DOWN*1 + RIGHT*1)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(right_array_group,LEFT,buff=0.1)
        rb.next_to(right_array_group,RIGHT,buff=0.1)
        right_group = VGroup(lb, right_array_group, rb)

        g = VGroup(left_group, right_group)
        self.play(Write(g))
        self.wait(1)

        arrow = Arrow(start = left_group.get_center()+DOWN*2, end = left_group.get_center()+DOWN*0.5)
        self.play(Write(arrow))
        self.wait(1)

        three.scale(0.5)
        three.next_to(five, LEFT*3+DOWN*2, buff=0.1)
        three_text.move_to(three.get_center())
        l = Line(five.get_center(), three_text.get_center())
        g = VGroup(l, three)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(left_array[1], three_text))
        self.wait(2)

        another_arrow = Arrow(start = right_group.get_center()+DOWN*2, end = right_group.get_center()+DOWN*0.5)
        self.play(Transform(arrow, another_arrow))
        self.wait(1)

        eight.scale(0.5)
        eight.next_to(five, RIGHT*3+DOWN*2, buff=0.1)
        eight_text.move_to(eight.get_center())
        l = Line(five.get_center(), eight_text.get_center())
        g = VGroup(l, eight)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(right_array[1], eight_text))
        self.wait(2)

        self.play(arrow.animate.set_opacity(0))

        one_numbers = [1]
        one_array = [Integer(num) for num in one_numbers]
        one_array_group = VGroup(*one_array).arrange(RIGHT, buff=0.5)
        one_array_group.move_to(left_array[0].get_center() + DOWN*1 + LEFT*0.75)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(one_array_group,LEFT,buff=0.1)
        rb.next_to(one_array_group,RIGHT,buff=0.1)
        one_group = VGroup(lb, one_array_group, rb)

        four_numbers = [4]
        four_array = [Integer(num) for num in four_numbers]
        four_array_group = VGroup(*four_array).arrange(RIGHT, buff=0.5)
        four_array_group.move_to(left_array[2].get_center() + DOWN*1 + RIGHT*0.75)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(four_array_group,LEFT,buff=0.1)
        rb.next_to(four_array_group,RIGHT,buff=0.1)
        four_group = VGroup(lb, four_array_group, rb)


        six_numbers = [6]
        six_array = [Integer(num) for num in six_numbers]
        six_array_group = VGroup(*six_array).arrange(RIGHT, buff=0.5)
        six_array_group.move_to(right_array[0].get_center() + DOWN*1 + LEFT*0.75)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(six_array_group,LEFT,buff=0.1)
        rb.next_to(six_array_group,RIGHT,buff=0.1)
        six_group = VGroup(lb, six_array_group, rb)

        nine_numbers = [9]
        nine_array = [Integer(num) for num in nine_numbers]
        nine_array_group = VGroup(*nine_array).arrange(RIGHT, buff=0.5)
        nine_array_group.move_to(right_array[2].get_center() + DOWN*1 + RIGHT*0.75)
        lb = Text("[")
        rb = Text("]")
        lb.next_to(nine_array_group,LEFT,buff=0.1)
        rb.next_to(nine_array_group,RIGHT,buff=0.1)
        nine_group = VGroup(lb, nine_array_group, rb)
        g = VGroup(one_group, four_group, six_group, nine_group)
        self.play(Write(g))
        self.wait(1)

        another_arrow = Arrow(start = one_group.get_center()+DOWN*2, end = one_group.get_center()+DOWN*0.5)
        self.play(Write(another_arrow))
        one.scale(0.5)
        one.next_to(three, LEFT*0.05+DOWN*2, buff=0.1)
        one_text.move_to(one.get_center())
        l = Line(three.get_center(), one_text.get_center())
        g = VGroup(l, one)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(one_array[0], one_text))
        self.bring_to_front(one_text)
        self.wait(1)

        nother_arrow = Arrow(start = four_group.get_center()+DOWN*2, end = four_group.get_center()+DOWN*0.5)
        self.play(Transform(another_arrow, nother_arrow))
        four.scale(0.5)
        four.next_to(three, RIGHT*0.05+DOWN*2, buff=0.1)
        four_text.move_to(four.get_center())
        l = Line(three.get_center(), four_text.get_center())
        g = VGroup(l, four)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(four_array[0], four_text))
        self.bring_to_front(four_text)
        self.wait(1)

        other_arrow = Arrow(start = six_group.get_center()+DOWN*2, end = six_group.get_center()+DOWN*0.5)
        self.play(Transform(another_arrow, other_arrow))
        six.scale(0.5)
        six.next_to(eight, LEFT*0.05+DOWN*2, buff=0.1)
        six_text.move_to(six.get_center())
        l = Line(eight.get_center(), six_text.get_center())
        g = VGroup(l, six)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(six_array[0], six_text))
        self.bring_to_front(six_text)
        self.wait(1)

        ther_arrow = Arrow(start = nine_group.get_center()+DOWN*2, end = nine_group.get_center()+DOWN*0.5)
        self.play(Transform(another_arrow, ther_arrow))
        nine.scale(0.5)
        nine.next_to(eight, RIGHT*0.05+DOWN*2, buff=0.1)
        nine_text.move_to(nine.get_center())
        l = Line(eight.get_center(), nine_text.get_center())
        g = VGroup(l, nine)
        self.bring_to_back(g)
        self.play(Write(g))
        self.play(Transform(nine_array[0], nine_text))
        self.bring_to_front(nine_text)
        self.wait(2)

        self.clear()

        self.play(self.camera.frame.animate.move_to(ORIGIN))

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

        # Display the array
        full_array.move_to(ORIGIN)
        self.play(Write(full_array))
        self.wait(2)

        arrow = Arrow(start = array_elements[2].get_center() + DOWN*2, end = array_elements[2].get_center() + DOWN*0.5, color = YELLOW)
        center = Text("size / 2", color = YELLOW)
        center_label = Text("Center Point", color = YELLOW)
        center.next_to(arrow, DOWN)
        center_label.next_to(center, DOWN)
        g = VGroup(arrow, center, center_label)
        self.play(array_elements[2].animate.set_color(YELLOW))
        self.play(Write(g))
        self.wait(1)

        self.play(array_elements[0].animate.set_color(RED))
        self.play(array_elements[1].animate.set_color(RED))
        lower = Text("Lower Half", color = RED)
        arrow = Arrow(start = (array_elements[1].get_center() + array_elements[0].get_center())/2 + UP*2+LEFT,
                      end = (array_elements[1].get_center() + array_elements[0].get_center())/2 + UP*0.5, color = RED)
        lower.next_to(arrow, UP+LEFT)
        g = VGroup(lower, arrow)
        self.play(Write(g))
        self.wait(1)

        self.play(array_elements[3].animate.set_color(PURE_BLUE))
        lower = Text("Upper Half", color = PURE_BLUE)
        arrow = Arrow(start = array_elements[3].get_center() + UP*2+RIGHT,
                      end = array_elements[3].get_center() + UP*0.5, color = PURE_BLUE)
        lower.next_to(arrow, UP+RIGHT)
        g = VGroup(lower, arrow)
        self.play(Write(g))
        self.wait(1)

        self.play(self.camera.frame.animate.move_to(ORIGIN+UP*2).set(width = 20))

        o = Text("1. Base cases, size==0 and size==1")
        t = Text("2. Center element is added to tree")
        th = Text("3. Recursively call on both left and right child")
        t.next_to(o,DOWN)
        th.next_to(t,DOWN)

        g = VGroup(o,t,th)
        g.move_to(ORIGIN+UP*6)
        self.play(Write(g))
        self.wait(1)

        self.clear()


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

        self.play(array_elements[2].animate.set_color(YELLOW))
        self.wait(1)

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
        self.play(Write(full_array))
        self.play(self.camera.frame.animate.move_to(ORIGIN+DOWN+LEFT).set(width=5))
        self.wait(1)

        self.play(array_elements[1].animate.set_color(YELLOW))

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
        self.play(array_elements[0].animate.set_color(RED))
        t = Text("Base Case (size 1")
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
