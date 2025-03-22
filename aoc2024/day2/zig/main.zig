const std = @import("std");
const file = @embedFile(".part1.txt");

fn valid(nums: []i32) bool {
    var dir: u2 = 3;
    var prev = nums[0];

    for (nums[1..]) |val| {
        if (dir == 3) {
            dir = @intFromBool(val > prev);
        }

        if (@intFromBool(val > prev) != dir) {
            return false;
        }

        const diff = @abs(val - prev);
        if (diff == 0 or diff > 3) {
            return false;
        }

        prev = val;
    }

    return true;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();
    var lines = std.mem.tokenizeScalar(u8, file, '\n');
    var acc: u32 = 0;

    outer: while (lines.next()) |line| {
        var al = std.ArrayList(i32).init(allocator);
        defer al.deinit();

        var nums = std.mem.tokenizeScalar(u8, line, ' ');
        while (nums.next()) |num| {
            const parsed = try std.fmt.parseInt(i32, num, 10);
            try al.append(parsed);
        }

        // Try it regular style
        if (valid(al.items)) {
            acc += 1;
            continue :outer;
        }

        for (0..al.items.len) |i| {
            // We will be skipping index i
            var inner_list = std.ArrayList(i32).init(allocator);
            defer inner_list.deinit();

            for (al.items, 0..) |val, idx| {
                if (idx == i) {
                    continue;
                }

                try inner_list.append(val);
            }

            if (valid(inner_list.items)) {
                acc += 1;
                continue :outer;
            }
        }
    }

    std.debug.print("{}\n", .{acc});
}
