const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const rows = std.mem.countScalar(u8, file, '\n');
    const cols = std.mem.findScalar(u8, file, '\n').?;
    const expanded_cols = cols + 2;

    var map = try allocator.alloc(bool, (rows + 2) * expanded_cols);
    var line_iter = std.mem.tokenizeScalar(u8, file, '\n');

    var row: usize = 0;
    while (line_iter.next()) |line| {
        for (line, 0..) |c, i| {
            map[((row + 1) * expanded_cols) + i + 1] = c == '@';
        }
        row += 1;
    }

    var count: usize = 0;
    for (0..100) |_| {
        var round_count: usize = 0;
        for (1..rows + 1) |i| {
            for (1..expanded_cols) |j| {
                const index = (i * expanded_cols) + j;
                if (map[index] != true) {
                    continue;
                }

                var neighbors: usize = 0;
                neighbors += if (map[index - expanded_cols - 1]) 1 else 0;
                neighbors += if (map[index - expanded_cols]) 1 else 0;
                neighbors += if (map[index - expanded_cols + 1]) 1 else 0;
                neighbors += if (map[index - 1]) 1 else 0;
                neighbors += if (map[index + 1]) 1 else 0;
                neighbors += if (map[index + expanded_cols - 1]) 1 else 0;
                neighbors += if (map[index + expanded_cols]) 1 else 0;
                neighbors += if (map[index + expanded_cols + 1]) 1 else 0;

                round_count += if (neighbors < 4) 1 else 0;
                map[index] = neighbors >= 4;
            }
        }
        if (round_count > 0) {
            count += round_count;
        } else {
            break;
        }
    }

    std.log.info("Count: {}\n", .{count});
}
