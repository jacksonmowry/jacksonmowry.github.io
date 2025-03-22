const std = @import("std");
const file = @embedFile("parts1.txt");

const Token = union(enum) {
    mul: void,
    l_paren: void,
    num: i32,
    comma: void,
    r_paren: void,
    junk: void,
};

pub fn main() !void {}
