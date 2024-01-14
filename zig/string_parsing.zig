const std = @import("std");
const expect = std.testing.expect;

fn parseString(comptime string: []const u8) usize {
    comptime {
        var i: usize = 0;

        for (string, 0..) |char, pos| {
            if (char == '$') i += 1;
            _ = pos;
        }

        return i;
    }
}

test "parsing string at comptime" {
    const count = comptime parseString("$$$$");
    try expect(count == 4);
}
