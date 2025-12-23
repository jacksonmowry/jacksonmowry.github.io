const std = @import("std");

const file = @embedFile(".input.txt");
const num_points = blk: {
    @setEvalBranchQuota(1000000);
    break :blk std.mem.countScalar(u8, file, '\n');
};

const Point = struct {
    const Self = @This();

    x: i64,
    y: i64,
    z: i64,

    leader: ?*Self,
    size: usize,

    fn distance(self: Self, other: Self) f64 {
        return @sqrt(@floatFromInt((self.x - other.x) * (self.x - other.x) + (self.y - other.y) * (self.y - other.y) + (self.z - other.z) * (self.z - other.z)));
    }

    fn rec(self: *Self) *Self {
        if (self.leader == null) {
            return self;
        } else {
            return rec(self.leader.?);
        }
    }

    fn parent(self: *Self) *Self {
        return rec(self);
    }

    fn merge(self: *Self, other: *Self) void {
        var a = self.parent();
        var b = other.parent();

        if (a == b) {
            return;
        }

        const leader = if (a.size >= b.size) a else b;
        const follower = if (leader == a) b else a;

        std.debug.assert(leader != follower);

        leader.size += follower.size;
        follower.size = 0;
        follower.leader = leader;
    }
};

const Pair = struct {
    a: *Point,
    b: *Point,
    distance: f64,

    fn lessThan(context: void, a: Pair, b: Pair) std.math.Order {
        _ = context;
        return std.math.order(a.distance, b.distance);
    }
};

fn gt(context: void, a: usize, b: usize) std.math.Order {
    _ = context;
    return std.math.order(b, a);
}

pub fn main() !void {
    var points: [num_points]Point = undefined;
    var points_len: usize = 0;

    var pairings_buf: [num_points * num_points * @sizeOf(Pair)]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&pairings_buf);
    const allocator = fba.allocator();
    var pq = std.PriorityQueue(Pair, void, Pair.lessThan).init(allocator, {});

    var iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (iter.next()) |line| {
        var line_iter = std.mem.tokenizeScalar(u8, line, ',');
        points[points_len].x = try std.fmt.parseInt(i64, line_iter.next().?, 0);
        points[points_len].y = try std.fmt.parseInt(i64, line_iter.next().?, 0);
        points[points_len].z = try std.fmt.parseInt(i64, line_iter.next().?, 0);
        points[points_len].leader = null;
        points[points_len].size = 1;
        points_len += 1;
    }

    for (0..points_len) |i| {
        for (i + 1..points_len) |j| {
            try pq.add(.{ .a = &points[i], .b = &points[j], .distance = points[i].distance(points[j]) });
        }
    }

    while (true) {
        const pair = pq.removeOrNull() orelse {
            break;
        };

        pair.a.merge(pair.b);
        if (pair.a.parent().size >= num_points or pair.b.parent().size >= num_points) {
            std.log.info("{}", .{
                pair.a.x * pair.b.x,
            });
            std.process.exit(0);
        }
    }

    pq.deinit();
}
