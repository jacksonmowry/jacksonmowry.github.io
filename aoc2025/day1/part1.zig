const std = @import("std");

const file = @embedFile(".part1.input");

pub fn main() !void {
    var pos: i32 = 50;
    var count: u32 = 0;

    var iter = std.mem.tokenizeAny(u8, file, "\n");
    while (iter.next()) |line| {
        const direction = line[0];
        var magnitude = try std.fmt.parseInt(i32, line[1..], 0);

        magnitude *= if (direction == 'L') -1 else 1;

        pos = @rem(pos + magnitude, 100);

        count += if (pos == 0) 1 else 0;
    }

    std.log.info("answer: {}\n", .{count});
}
