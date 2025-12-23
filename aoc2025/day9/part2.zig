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
    var points: [num_points + 1]Point = undefined;
    var points_len: usize = 0;

    var file_iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (file_iter.next()) |line| {
        var line_iter = std.mem.tokenizeScalar(u8, line, ',');
        points[points_len].x = try std.fmt.parseInt(isize, line_iter.next().?, 0);
        points[points_len].y = try std.fmt.parseInt(isize, line_iter.next().?, 0);
        points_len += 1;
    }
    points[num_points] = points[0];

    var max_area: u64 = 0;

    for (0..points_len) |i| {
        for (i + 1..points_len) |j| {
            const area = (@abs(points[i].x - points[j].x) + 1) * (@abs(points[i].y - points[j].y) + 1);
            if (area > max_area) {
                var line_segment_iter = std.mem.window(Point, &points, 2, 1);
                const valid = while (line_segment_iter.next()) |l| {
                    const p1: Point = .{ .x = @min(l[0].x, l[1].x), .y = @min(l[0].y, l[1].y) };
                    const p2: Point = .{ .x = @max(l[0].x, l[1].x), .y = @max(l[0].y, l[1].y) };
                    const r1: Point = .{ .x = @min(points[i].x, points[j].x), .y = @min(points[i].y, points[j].y) };
                    const r2: Point = .{ .x = @max(points[i].x, points[j].x), .y = @max(points[i].y, points[j].y) };

                    if (p2.x > r1.x and p1.x < r2.x and p2.y > r1.y and p1.y < r2.y) {
                        break false;
                    }
                } else true;

                if (valid) {
                    max_area = area;
                }
            }
        }
    }

    std.log.info("Answer: {}", .{max_area});
}
