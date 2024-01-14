const std = @import("std");

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();
    var i: u8 = 1;
    while (i <= 100) : (i += 1) {
        const div_3: u2 = @intFromBool(i % 3 == 0);
        const div_5 = @intFromBool(i % 5 == 0);

        switch (div_3 << 1 | div_5) {
            0b10 => try stdout.writeAll("Fizz\n"),
            0b01 => try stdout.writeAll("Buzz\n"),
            0b11 => try stdout.writeAll("FizzBuzz\n"),
            0b00 => try stdout.print("{}\n", .{i}),
        }
    }
}
