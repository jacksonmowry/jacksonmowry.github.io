const std = @import("std");

const file = @embedFile(".input.txt");

pub fn dfs(row: usize, col: usize, width: usize, cache: []usize) usize {
    var new_row = row;

    while (new_row * width + col < file.len and (file[new_row * width + col] == '.' or file[new_row * width + col] == 'S')) {
        new_row += 1;
    }

    if (new_row * width + col >= file.len) {
        return 0;
    }

    if (cache[new_row * width + col] != std.math.maxInt(usize)) {
        return 0;
        // return cache[new_row * width + col];
    }

    var count: usize = 1;
    if (col - 1 >= 0) {
        count += dfs(new_row, col - 1, width, cache);
    }
    if (col + 1 < width - 1) {
        count += dfs(new_row, col + 1, width, cache);
    }

    cache[new_row * width + col] = count;
    return count;
}

pub fn main() !void {
    const cols = std.mem.findScalar(u8, file, '\n').? + 1;

    var cache = [_]usize{std.math.maxInt(usize)} ** file.len;

    const col: usize = std.mem.findScalar(u8, file, 'S').?;
    const row: usize = 0;

    const paths = dfs(row, col, cols, cache[0..]);
    std.log.info("Answer: {}", .{paths});
}
