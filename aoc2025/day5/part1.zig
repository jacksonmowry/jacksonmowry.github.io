const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var count: usize = 0;

    var halves = std.mem.splitSequence(u8, file, "\n\n");

    var ranges_iter = std.mem.tokenizeScalar(u8, halves.next().?, '\n');
    var id_iter = std.mem.tokenizeScalar(u8, halves.next().?, '\n');

    var lower: [512]usize = undefined;
    var upper: [512]usize = undefined;
    var num_ranges: usize = 0;

    while (ranges_iter.next()) |line| {
        var line_iter = std.mem.tokenizeScalar(u8, line, '-');
        lower[num_ranges] = try std.fmt.parseInt(usize, line_iter.next().?, 0);
        upper[num_ranges] = try std.fmt.parseInt(usize, line_iter.next().?, 0);
        num_ranges += 1;
    }

    while (id_iter.next()) |line| {
        const val = try std.fmt.parseInt(usize, line, 0);

        for (0..num_ranges) |i| {
            if (val >= lower[i] and val <= upper[i]) {
                count += 1;
                break;
            }
        }
    }

    std.log.info("Answer: {}", .{count});
}
