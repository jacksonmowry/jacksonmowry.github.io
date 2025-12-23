const std = @import("std");

const file = @embedFile(".input.txt");

pub fn main() !void {
    var sum: usize = 0;
    var prod: usize = 1;

    var answer: usize = 0;

    const cols = std.mem.findScalar(u8, file, '\n').? + 1;
    const rows = std.mem.countScalar(u8, file, '\n');
    var col: isize = @intCast(cols - 2);

    while (col >= 0) : (col -= 1) {
        var num: usize = 0;

        const tmp_col: usize = @intCast(col);
        for (0..rows - 1) |row| {
            switch (file[(row * cols) + tmp_col]) {
                '0'...'9' => {
                    num *= 10;
                    num += file[(row * cols) + tmp_col] - '0';
                },
                else => continue,
            }
        }

        sum += num;
        prod *= if (num == 0) 1 else num;

        if (num != 0) {
            std.log.debug("Parsed {}", .{num});
        }

        if (file[(rows - 1) * cols + tmp_col] != ' ') {
            // Op
            if (file[(rows - 1) * cols + tmp_col] == '+') {
                answer += sum;
                std.log.info("sum {}, answer {}", .{ sum, answer });
            } else {
                std.log.info("pro {}, answer {}", .{ prod, answer });
                answer += prod;
            }

            sum = 0;
            prod = 1;
        }
    }

    std.log.info("Answer: {}", .{answer});
}
