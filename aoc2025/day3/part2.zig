const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var sum: usize = 0;

    var line_iter = std.mem.tokenizeAny(u8, file, "\n");
    while (line_iter.next()) |line| {
        var offsets: [12]usize = undefined;
        for (0..12) |i| {
            const offset = if (i == 0) 0 else offsets[i - 1] + 1;

            offsets[i] = std.mem.findMax(u8, line[offset..(line.len - (11 - i))]) + offset;
        }

        var local_sum: usize = 0;
        for (offsets) |idx| {
            local_sum *= 10;
            local_sum += line[idx] - '0';
        }
        sum += local_sum;
    }

    std.log.info("Answer: {}", .{sum});
}
