const std = @import("std");
const file = @embedFile("part1.txt");
const ArrayList = std.ArrayList;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();

    var matrix = ArrayList(ArrayList(u8)).init(allocator);
    defer matrix.deinit();

    var lines = std.mem.tokenizeScalar(u8, file, '\n');
    while (lines.next()) |line| {
        try matrix.append(ArrayList(u8).init(allocator));
        var last_al = &matrix.items[matrix.items.len - 1];

        for (line) |c| {
            try last_al.append(c);
        }
    }

    for (0..matrix.items.len) |row| {
        for (0..matrix.items[0].items.len) |col| {}
    }

    // Cleanup rows
    for (matrix.items) |*row| {
        row.deinit();
    }
}
