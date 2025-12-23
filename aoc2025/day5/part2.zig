const std = @import("std");

const file = @embedFile(".input.txt");

const Pair = struct {
    lower: usize,
    upper: usize,

    fn lessThan(context: void, a: Pair, b: Pair) bool {
        _ = context;
        if (a.lower == b.lower) {
            return a.upper < b.upper;
        }
        return a.lower < b.lower;
    }
};

pub fn main() !void {
    var count: usize = 0;

    var halves = std.mem.splitSequence(u8, file, "\n\n");

    var ranges_iter = std.mem.tokenizeScalar(u8, halves.next().?, '\n');

    var ranges: [512]Pair = undefined;
    var num_ranges: usize = 0;

    while (ranges_iter.next()) |line| {
        var line_iter = std.mem.tokenizeScalar(u8, line, '-');
        ranges[num_ranges].lower = try std.fmt.parseInt(usize, line_iter.next().?, 0);
        ranges[num_ranges].upper = try std.fmt.parseInt(usize, line_iter.next().?, 0);
        num_ranges += 1;
    }

    std.sort.pdq(Pair, ranges[0..num_ranges], {}, Pair.lessThan);

    for (0..num_ranges - 1) |i| {
        if (ranges[i].upper >= ranges[i + 1].lower and ranges[i].lower <= ranges[i + 1].upper) {
            ranges[i + 1].lower = @min(ranges[i].lower, ranges[i + 1].lower);
            ranges[i + 1].upper = @max(ranges[i].upper, ranges[i + 1].upper);
        } else {
            // No overlap
            count += ranges[i].upper - ranges[i].lower + 1;
        }
    }

    count += ranges[num_ranges - 1].upper - ranges[num_ranges - 1].lower + 1;

    std.log.info("Answer: {}", .{count});
}
