const std = @import("std");

pub fn main() !void {
    var charges = @Vector(8, u8){ 1, 2, 3, 5, 4, 2, 1, 9 };
    const thresholds = @Vector(8, u8){ 4, 2, 1, 5, 7, 3, 7, 9 };

    const tmp: [8]u8 = [_]u8{ 1, 2, 3, 4, 5, 6, 7, 8 };

    charges = tmp;

    const fired = charges >= thresholds;
    const fired_arr: [8]bool = fired;

    for (fired_arr, 0..) |f, i| {
        if (f) {
            std.debug.print("Neuron {} fired.\n", .{i});
        }
    }
}
