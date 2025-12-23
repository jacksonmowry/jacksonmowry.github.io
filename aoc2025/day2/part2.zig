const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var count: usize = 0;
    var iter = std.mem.tokenizeAny(u8, file, ",\n");
    while (iter.next()) |range| {
        var inner_iter = std.mem.tokenizeAny(u8, range, "-");
        const l = inner_iter.next().?;
        const u = inner_iter.next().?;

        errdefer {
            std.log.err("Failed while parsing {s} and {s}\n", .{ l, u });
        }

        const lower = try std.fmt.parseInt(u64, l, 0);
        const upper = try std.fmt.parseInt(u64, u, 0);

        for (lower..upper + 1) |num| {
            var stack_buf: [32]u8 = undefined;
            const slice = try std.fmt.bufPrint(&stack_buf, "{}", .{num});

            const half_len = (slice.len) / 2;
            var invalid_password = false;
            for (1..half_len + 1) |slice_len| {
                if (slice.len % slice_len != 0) {
                    continue;
                }

                var innermost_iter = std.mem.window(u8, slice, slice_len, slice_len);
                const first = innermost_iter.next().?;

                var valid = true;
                while (innermost_iter.next()) |s| {
                    if (!std.mem.eql(u8, first, s)) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    invalid_password = true;
                    break;
                }
            }

            if (invalid_password) {
                count += num;
            }
        }
    }

    std.log.info("answer: {}\n", .{count});
}
