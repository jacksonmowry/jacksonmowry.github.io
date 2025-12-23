const std = @import("std");

const file = @embedFile(".part1.input");

pub fn main() !void {
    var pos: i32 = 50;
    var count: u32 = 0;

    var iter = std.mem.tokenizeAny(u8, file, "\n");
    while (iter.next()) |line| {
        const direction = line[0];
        const magnitude = try std.fmt.parseInt(usize, line[1..], 0);

        for (0..magnitude) |_| {
            const new_val = pos + if (direction == 'L') @as(i32, -1) else 1;
            pos = @rem(new_val, 100);
            count += if (pos == 0) 1 else 0;
        }
    }

    std.log.info("answer: {}\n", .{count});
}
