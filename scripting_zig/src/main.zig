const std = @import("std");
const Token = @import("Token.zig").Token;
const Tokenizer = @import("Tokenizer.zig").Tokenizer;
const Allocator = std.mem.Allocator;

pub fn main() !void {
    var debug_allocator: std.heap.DebugAllocator(.{}) = .init;
    const gpa = debug_allocator.allocator();
    defer _ = debug_allocator.deinit();

    var args = try std.process.argsWithAllocator(gpa);
    if (!args.skip()) {
        std.debug.print("No .sc file provided!\n", .{});
        std.process.exit(1);
    }

    const input_file_name = args.next() orelse {
        std.debug.print("Not enough arguments provided\n", .{});
        std.process.exit(1);
    };

    if (args.skip()) {
        std.debug.print("Expected 1 .sc file, got more than 1\n", .{});
        std.process.exit(1);
    }

    var cwd = std.fs.cwd();
    var input_file = try cwd.openFile(input_file_name, .{});
    defer input_file.close();

    var buf: [100]u8 = [_]u8{0} ** 100;
    var input_file_reader = input_file.reader(&buf);

    var arena = std.heap.ArenaAllocator.init(gpa);
    defer arena.deinit();
    const arena_allocator = arena.allocator();

    var tokenizer: Tokenizer = .{
        .input = &input_file_reader.interface,
        .tokens = std.ArrayListUnmanaged(Token){},
    };
    defer tokenizer.deinit(arena_allocator);

    try tokenizer.tokenize(arena_allocator);

    for (tokenizer.tokens.items) |tok| {
        tok.debugPrint();
    }

    // Once we're done with the tokenizer we can clear its arena
    defer _ = arena.reset(.free_all);

    std.debug.print("{any}\n", .{tokenizer.tokens.items});
    // Prints to stderr, ignoring potential errors.
    // std.debug.print("All your {s} are belong to us.\n", .{"codebase"});
    // try scripting_zig.bufferedPrint();
}
