const std = @import("std");

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();
    const stdin = std.io.getStdOut().reader();
    var prng = std.rand.DefaultPrng.init(blk: {
        var seed: u64 = undefined;
        try std.os.getrandom(std.mem.asBytes(&seed));
        break :blk seed;
    });
    const rand = prng.random();
    const num = rand.intRangeAtMost(u8, 0, 255);
    while (true) {
        try stdout.print("Guess the number: ", .{});
        const bare_line = try stdin.readUntilDelimiterAlloc(
            std.heap.page_allocator,
            '\n',
            8192,
        );
        defer std.heap.page_allocator.free(bare_line);

        const guess = std.fmt.parseInt(u8, bare_line, 10) catch |err| switch (err) {
            error.Overflow => {
                try stdout.writeAll("Please enter a small postive number\n");
                continue;
            },
            error.InvalidCharacter => {
                try stdout.writeAll("Please enter a valid number\n");
                continue;
            },
        };

        if (guess < num) try stdout.writeAll("Too small!\n");
        if (guess > num) try stdout.writeAll("Too big!\n");
        if (guess == num) {
            try stdout.writeAll("You got it!\n");
            break;
        }
    }
}
