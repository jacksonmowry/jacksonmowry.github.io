const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var count: usize = 0;
    var line_iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (line_iter.next()) |line| {
        var hash_iter = std.mem.tokenizeAny(u8, line, "[]");
        var lights: u16 = 0;
        for (hash_iter.next().?, 0..) |c, i| {
            lights |= (if (c == '#') @as(u16, 1) else 0) << @intCast(i);
        }

        var rest_iter = std.mem.tokenizeScalar(u8, line, ' ');
        _ = rest_iter.next().?;

        var ops: [16]u16 = undefined;
        var row: u4 = 0;
        while (rest_iter.next()) |atom| {
            if (atom[0] == '{') {
                break;
            }

            var acc: u16 = 0;

            var atom_iter = std.mem.tokenizeAny(u8, atom, "(,)");
            while (atom_iter.next()) |num| {
                const index: u4 = try std.fmt.parseInt(u4, num, 0);
                acc |= @as(u16, 1) << index;
            }

            ops[row] = acc;

            row += 1;
        }

        var min: usize = std.math.maxInt(usize);
        for (0..@as(u16, 1) << row) |i| {
            var tmp_lights: u16 = 0;

            for (0..row) |j| {
                const shft_amt: u6 = @intCast(j);
                if (((i >> shft_amt) & 1) == 1) {
                    tmp_lights ^= ops[j];
                }
            }

            if (tmp_lights == lights and @popCount(i) < min) {
                min = @popCount(i);
            }
        }

        count += min;
    }

    std.log.debug("Ops: {}", .{count});
}
