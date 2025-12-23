const std = @import("std");

const file = @embedFile(".input.txt");
const num_points = blk: {
    @setEvalBranchQuota(100000);
    break :blk std.mem.countScalar(u8, file, '\n');
};

const Point = struct {
    x: isize,
    y: isize,
};

pub fn main() !void {
    var points: [num_points]Point = undefined;
    var points_len: usize = 0;

    var file_iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (file_iter.next()) |line| {
        var line_iter = std.mem.tokenizeScalar(u8, line, ',');
        points[points_len].x = try std.fmt.parseInt(isize, line_iter.next().?, 0);
        points[points_len].y = try std.fmt.parseInt(isize, line_iter.next().?, 0);
        points_len += 1;
    }

    var max_area: u64 = 0;

    for (0..points_len) |i| {
        for (i + 1..points_len) |j| {
            const area = (@abs(points[i].x - points[j].x) + 1) * (@abs(points[i].y - points[j].y) + 1);
            if (area > max_area) {
                max_area = area;
            }
        }
    }

    std.log.info("Answer: {}", .{max_area});
}
