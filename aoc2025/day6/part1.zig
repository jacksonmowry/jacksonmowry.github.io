const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var sum = [_]usize{0} ** 4096;
    var prod = [_]usize{1} ** 4096;

    var answer: usize = 0;

    var line_iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (line_iter.next()) |line| {
        var col: usize = 0;
        var iter = std.mem.tokenizeScalar(u8, line, ' ');

        if (line[0] == '*' or line[0] == '+') {
            while (iter.next()) |symb| {
                switch (symb[0]) {
                    '+' => answer += sum[col],
                    '*' => answer += prod[col],
                    else => unreachable,
                }

                col += 1;
            }
        } else {
            while (iter.next()) |n| {
                const num = try std.fmt.parseInt(usize, n, 0);

                sum[col] += num;
                prod[col] *= num;

                col += 1;
            }
        }
    }

    std.log.info("Answer: {}", .{answer});
}
