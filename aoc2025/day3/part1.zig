const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var sum: usize = 0;

    var line_iter = std.mem.tokenizeAny(u8, file, "\n");
    while (line_iter.next()) |line| {
        const first_bat = std.mem.findMax(u8, line[0 .. line.len - 1]);
        const second_bat = std.mem.findMax(u8, line[first_bat + 1 ..]) + first_bat + 1;

        std.log.debug("Batteries {c}{c} @ {}{}\n", .{ line[first_bat], line[second_bat], first_bat, second_bat });
        sum += ((line[first_bat] - '0') * 10) + (line[second_bat] - '0');
    }

    std.log.info("Answer: {}", .{sum});
}
