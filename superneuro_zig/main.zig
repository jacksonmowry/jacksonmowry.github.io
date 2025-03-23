const std = @import("std");
const sn = @import("./superneuro.zig");
const Instant = std.time.Instant;
const rand = std.crypto.random;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    var nm = try sn.NeuromorphicModel.init(allocator);
    defer nm.deinit();

    for (0..256) |_| {
        _ = try nm.create_neuron(1, 1, 0, 0);
    }

    for (0..128) |i| {
        for (0..32) |j| {
            _ = try nm.create_synapse(@intCast(i), @intCast(128 + ((i + j) % 128)), 1, 1, false);
        }
    }

    try nm.setup();

    var total_time: u64 = 0;

    for (0..1000) |_| {
        for (0..128) |input| {
            if (rand.uintLessThan(u8, 100) + 1 < 25) {
                try nm.add_spike(@intCast(0), @intCast(input), 1);
            }
        }

        const before = try Instant.now();
        try nm.simulate(1);
        const after = try Instant.now();

        total_time += after.since(before);
    }

    var res: f64 = @floatFromInt(total_time);
    res /= std.time.ns_per_s;
    res /= 1000;
    res = 1 / res;
    std.debug.print("FPS: {}\n", .{res});
}
