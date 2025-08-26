const std = @import("std");
const Allocator = std.mem.Allocator;

const Token = @import("Token.zig").Token;

pub const Tokenizer = struct {
    input: *std.Io.Reader,
    tokens: std.ArrayListUnmanaged(Token),

    const Self = @This();

    pub fn tokenize(self: *Self, allocator: Allocator) !void {
        while (true) {
            const token = nextToken(self.input, allocator, .outside) catch break;
            if (token) |t| {
                try self.tokens.append(allocator, t);
            }
        }
    }

    pub fn deinit(self: *Self, allocator: Allocator) void {
        for (self.tokens.items) |*token| {
            token.deinit(allocator);
        }

        self.tokens.deinit(allocator);
    }
};

test "Tokenizer.tokenize" {
    const string = "Hi mom this is a test {431.431 * 431 / 14}*1234";
    var r: std.Io.Reader = .fixed(string);

    var tokenizer: Tokenizer = .{ .input = &r, .tokens = std.ArrayListUnmanaged(Token){} };
    defer tokenizer.deinit(std.testing.allocator);

    const expected = [_]Token{
        Token{ .identifier = .{ .value = "Hi", .length = 2 } },
        Token{ .identifier = .{ .value = "mom", .length = 3 } },
        Token{ .identifier = .{ .value = "this", .length = 4 } },
        Token{ .identifier = .{ .value = "is", .length = 2 } },
        Token{ .identifier = .{ .value = "a", .length = 1 } },
        Token{ .identifier = .{ .value = "test", .length = 4 } },
        .l_brace,
        Token{ .real = .{ .value = 431.431, .length = 7 } },
        .multiplication,
        Token{ .integer = .{ .value = 431, .length = 3 } },
        .division,
        Token{ .integer = .{ .value = 14, .length = 2 } },
        .r_brace,
        .multiplication,
        Token{ .integer = .{ .value = 1234, .length = 4 } },
    };

    try tokenizer.tokenize(std.testing.allocator);

    try std.testing.expectEqualDeep(expected[0..], tokenizer.tokens.items);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

const WithinTemplateLiteral = enum {
    outside,
    text,
    template,
};

fn nextToken(
    reader: *std.Io.Reader,
    allocator: Allocator,
    inside_template_literal: WithinTemplateLiteral,
) !?Token {
    try consumeWhitespace(reader);
    const b = try reader.peekByte();
    const t: ?Token = switch (b) {
        '(' => .l_paren,
        ')' => .r_paren,
        '{' => .l_brace,
        '}' => .r_brace,
        '[' => .l_bracket,
        ']' => .r_bracket,
        '+' => .plus,
        '-' => .minus,
        '/' => .division,
        '*' => .multiplication,
        '<' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], "<=")) {
                break :blk .le;
            } else {
                break :blk .lt;
            }
        },
        '>' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], ">=")) {
                break :blk .ge;
            } else {
                break :blk .gt;
            }
        },
        '=' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], "==")) {
                break :blk .eq;
            } else {
                break :blk .assignment;
            }
        },
        '!' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], "!=")) {
                break :blk .neq;
            } else {
                break :blk .logical_not;
            }
        },
        '&' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], "&&")) {
                break :blk .logical_and;
            } else {
                break :blk .bitwise_and;
            }
        },
        '|' => blk: {
            const slice: []const u8 = try reader.peek(2);
            if (std.mem.eql(u8, slice[0..], "||")) {
                break :blk .logical_or;
            } else {
                break :blk .bitwise_or;
            }
        },
        '~' => .bitwise_xor,
        '.' => .dot,
        ',' => .comma,
        ':' => .colon,
        ';' => .semicolon,
        else => blk: {
            break :blk try tokenizeMultichar(reader, allocator, inside_template_literal);
        },
    };

    if (t) |token| {
        const toss_len = token.tokenLen();
        reader.toss(toss_len);

        return token;
    }

    return null;
}

test "nextToken" {
    var r: std.Io.Reader = .fixed("'9'1");

    // '9'
    const t1: Token = try nextToken(&r, std.testing.allocator, .outside) orelse undefined;
    defer t1.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .char_literal = '9' }, t1);

    // 1
    const t2: Token = try nextToken(&r, std.testing.allocator, .outside) orelse undefined;
    defer t2.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .integer = .{ .value = 1, .length = 1 } }, t2);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

fn consumeWhitespace(reader: *std.Io.Reader) !void {
    var c = try reader.peekByte();
    while (c == ' ' or c == '\t' or c == '\n') {
        reader.toss(1);
        c = try reader.peekByte();
    }
}

test "consumeWhitespace" {
    var r: std.io.Reader = .fixed("        ");
    try std.testing.expectError(std.Io.Reader.Error.EndOfStream, consumeWhitespace(&r));
}

test "consumeWhitespace remaining chars" {
    var r: std.io.Reader = .fixed("       a");
    try consumeWhitespace(&r);

    const buf = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(buf);
    try std.testing.expectEqual(1, buf.len);
}

fn tokenizeMultichar(
    reader: *std.Io.Reader,
    allocator: Allocator,
    inside_template_literal: WithinTemplateLiteral,
) anyerror!Token {
    if (inside_template_literal == .text) {
        return tokenizeTemplateText(reader, allocator);
    }

    return switch (try reader.peekByte()) {
        '\'' => tokenizeChar(reader),
        '"' => tokenizeString(reader, allocator),
        '`' => blk: {
            if (inside_template_literal == .outside) {
                break :blk tokenizeTemplateString(reader, allocator);
            } else {
                return error.NestedTemplateLiterals;
            }
        },
        'a'...'z', 'A'...'Z' => tokenizeIdentifierOrKeyword(reader, allocator),
        '0'...'9' => tokenizeNumber(reader, allocator),
        else => unreachable,
    };
}

test "tokenizeMultichar text" {
    const string = "4312.431 fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeMultichar(&r, std.testing.allocator, .text);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .string_literal = .{ .value = string, .length = string.len } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

test "tokenizeMultichar nested template literal" {
    // This is the same as `some text ${`another`}`
    // We're just starting from within the first template block
    const string = "`another`";
    var r: std.Io.Reader = .fixed(string);

    try std.testing.expectError(error.NestedTemplateLiterals, tokenizeMultichar(&r, std.testing.allocator, .template));
}

test "tokenizeMultichar template literal" {
    // This is the same as `some text ${`another`}`
    // We're just starting from within the first template block
    const string = "`another`";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeMultichar(&r, std.testing.allocator, .outside);
    defer t.deinit(std.testing.allocator);

    var tokens = [_]Token{
        Token{ .string_literal = .{
            .value = "another",
            .length = 7,
        } },
    };

    try std.testing.expectEqualDeep(
        Token{ .template_string = .{ .tokens = &tokens, .length = 1 } },
        t,
    );
}

fn tokenizeChar(reader: *std.Io.Reader) !Token {
    var buf = try reader.peek(3);

    // We must start with a '
    std.debug.assert(buf[0] == '\'');

    const b: u8 = switch (buf[1]) {
        '\\' => blk: {
            // Grab the last char from the buffer as this sequence is 4 bytes long
            buf = try reader.peek(4);
            reader.toss(1);
            switch (buf[2]) {
                't' => break :blk '\t',
                'n' => break :blk '\n',
                else => return error.UnsupportedEscapseSequence,
            }
        },
        else => |c| c,
    };

    const c = try reader.take(3);
    if (c[2] != '\'') {
        return error.UnterminatedCharLiteral;
    }

    return Token{ .char_literal = b };
}

test "tokenizeChar letter" {
    var r: std.Io.Reader = .fixed("'a'");

    const t: Token = try tokenizeChar(&r);
    try std.testing.expectEqualDeep(Token{ .char_literal = 'a' }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    try std.testing.expectEqual(0, remaining.len);
}

test "tokenizeChar number" {
    var r: std.Io.Reader = .fixed("'9'1");

    const t: Token = try tokenizeChar(&r);
    try std.testing.expectEqualDeep(Token{ .char_literal = '9' }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(1, remaining.len);
}

test "tokenizeChar escaped char" {
    var r: std.Io.Reader = .fixed("'\\t'");

    const t: Token = try tokenizeChar(&r);
    try std.testing.expectEqualDeep(Token{ .char_literal = '\t' }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

fn tokenizeString(reader: *std.Io.Reader, allocator: Allocator) !Token {
    // We're assuming the first byte is a "
    std.debug.assert(try reader.peekByte() == '"');
    reader.toss(1);

    var buf = std.ArrayListUnmanaged(u8){};

    while (true) {
        const byte = reader.peekByte() catch break;
        switch (byte) {
            '"' => break,
            '\\' => {
                reader.toss(1);
                const escaped_char = try reader.peekByte();
                const escape_code: u8 = switch (escaped_char) {
                    't' => '\t',
                    'n' => '\n',
                    else => return error.UnknownEscapeSequence,
                };

                try buf.append(allocator, escape_code);
            },
            else => try buf.append(allocator, byte),
        }

        reader.toss(1);
    }

    std.debug.assert(try reader.peekByte() == '"');
    reader.toss(1);

    const buf_len = buf.items.len;
    return Token{ .string_literal = .{ .value = try buf.toOwnedSlice(allocator), .length = buf_len } };
}

test "tokenizeString" {
    const string = "\"this is a 'string \t \n 431 4312.123\"";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeString(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .string_literal = .{ .value = string[1 .. string.len - 1], .length = string.len - 2 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

fn tokenizeTemplateText(reader: *std.Io.Reader, allocator: Allocator) !Token {
    var buf = std.ArrayListUnmanaged(u8){};

    while (true) {
        const byte = reader.peekByte() catch break;
        switch (byte) {
            '"' => break,
            '\\' => {
                reader.toss(1);
                const escaped_char = try reader.peekByte();
                const escape_code: u8 = switch (escaped_char) {
                    't' => '\t',
                    'n' => '\n',
                    else => return error.UnknownEscapeSequence,
                };

                try buf.append(allocator, escape_code);
            },
            '`' => break,
            '$' => {
                if (std.mem.eql(u8, "${", try reader.peek(2))) {
                    break;
                } else {
                    try buf.append(allocator, byte);
                }
            },
            else => try buf.append(allocator, byte),
        }

        reader.toss(1);
    }

    const buf_len = buf.items.len;
    return Token{ .string_literal = .{ .value = try buf.toOwnedSlice(allocator), .length = buf_len } };
}

test "tokenizeTemplateText" {
    const string = "this is a 'string \t \n 431 4312.123";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeTemplateText(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .string_literal = .{ .value = string[0..], .length = string.len } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

fn tokenizeTemplateString(reader: *std.Io.Reader, allocator: Allocator) !Token {
    var tokens = std.ArrayListUnmanaged(Token){};
    var buf = std.ArrayListUnmanaged(u8){};
    defer buf.deinit(allocator);

    std.debug.assert(try reader.takeByte() == '`');

    while (true) {
        const b = try reader.peekByte();

        switch (b) {
            '$' => {
                // Check if we're within a template block
                const two = try reader.peek(2);
                if (!std.mem.eql(u8, "${", two)) {
                    try buf.append(allocator, try reader.takeByte());
                    continue;
                }

                // We're now within a template block
                // Flush buffer if present
                if (buf.items.len != 0) {
                    const buf_len = buf.items.len;
                    try tokens.append(allocator, Token{ .string_literal = .{
                        .value = try buf.toOwnedSlice(allocator),
                        .length = buf_len,
                    } });
                }
                reader.toss(2);

                while (true) {
                    const t: Token = try nextToken(reader, allocator, .template) orelse unreachable;

                    switch (t) {
                        .r_brace => break,
                        else => try tokens.append(allocator, t),
                        // Could probably do something about an
                        // unterminated template literal here
                    }
                }
            },
            '`' => break,
            else => {
                const t: Token = try tokenizeMultichar(reader, allocator, .text);
                try tokens.append(allocator, t);
            },
        }
    }

    reader.toss(1);

    const tokens_len = tokens.items.len;
    return Token{ .template_string = .{ .tokens = try tokens.toOwnedSlice(allocator), .length = tokens_len } };
}

test "tokenizeTemplateLiteral" {
    const string = "`Hi mom ${this} is a test`*1234";
    var r: std.Io.Reader = .fixed(string);

    var expected = [_]Token{
        Token{ .string_literal = .{ .value = "Hi mom ", .length = 7 } },
        Token{ .identifier = .{ .value = "this", .length = 4 } },
        Token{ .string_literal = .{ .value = " is a test", .length = 10 } },
    };

    const t: Token = try tokenizeTemplateString(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);

    try std.testing.expectEqualDeep(
        Token{ .template_string = .{ .tokens = &expected, .length = expected.len } },
        t,
    );

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(5, remaining.len);
}

test "tokenizeTemplateLiteral multi" {
    const string = "`Hi mom ${this} is a test ${431.431 * 431 / 14}`*1234";
    var r: std.Io.Reader = .fixed(string);

    var expected = [_]Token{
        Token{ .string_literal = .{ .value = "Hi mom ", .length = 7 } },
        Token{ .identifier = .{ .value = "this", .length = 4 } },
        Token{ .string_literal = .{ .value = " is a test ", .length = 11 } },
        Token{ .real = .{ .value = 431.431, .length = 7 } },
        .multiplication,
        Token{ .integer = .{ .value = 431, .length = 3 } },
        .division,
        Token{ .integer = .{ .value = 14, .length = 2 } },
    };

    const t: Token = try tokenizeTemplateString(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);

    try std.testing.expectEqualDeep(
        Token{ .template_string = .{ .tokens = &expected, .length = expected.len } },
        t,
    );

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(5, remaining.len);
}

fn tokenizeIdentifierOrKeyword(reader: *std.Io.Reader, allocator: Allocator) !Token {
    var buf = std.ArrayListUnmanaged(u8){};
    defer buf.deinit(allocator);

    while (true) {
        const b = try reader.peekByte();

        switch (b) {
            'a'...'z', 'A'...'Z', '_' => try buf.append(allocator, b),
            else => break,
        }

        reader.toss(1);
    }

    const buf_len = buf.items.len;
    std.debug.assert(buf_len > 0);

    if (std.mem.eql(u8, "for", buf.items)) {
        return .keyword_for;
    } else if (std.mem.eql(u8, "while", buf.items)) {
        return .keyword_while;
    } else if (std.mem.eql(u8, "const", buf.items)) {
        return .keyword_const;
    } else if (std.mem.eql(u8, "var", buf.items)) {
        return .keyword_var;
    } else if (std.mem.eql(u8, "return", buf.items)) {
        return .keyword_return;
    } else if (std.mem.eql(u8, "function", buf.items)) {
        return .keyword_function;
    } else if (std.mem.eql(u8, "continue", buf.items)) {
        return .keyword_continue;
    } else if (std.mem.eql(u8, "break", buf.items)) {
        return .keyword_break;
    } else if (std.mem.eql(u8, "defer", buf.items)) {
        return .keyword_defer;
    }

    return Token{ .identifier = .{ .value = try buf.toOwnedSlice(allocator), .length = buf_len } };
}

test "tokenizeIdentifierOrKeyword" {
    const string = "f_a 4321";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeIdentifierOrKeyword(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .identifier = .{ .value = string[0..3], .length = 3 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(5, remaining.len);
}

test "tokenizeIdentifierOrKeyword no whitespace" {
    const string = "f_a4321";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeIdentifierOrKeyword(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .identifier = .{ .value = string[0..3], .length = 3 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(4, remaining.len);
}

test "tokenizeIdentifierOrKeyword keyword" {
    const string = "for ()";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeIdentifierOrKeyword(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(.keyword_for, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(3, remaining.len);
}

fn tokenizeNumber(reader: *std.Io.Reader, allocator: Allocator) !Token {
    var buf = std.ArrayListUnmanaged(u8){};
    defer buf.deinit(allocator);
    var sep_chars: usize = 0;

    while (true) {
        const b = reader.peekByte() catch break;

        switch (b) {
            '0'...'9',
            'A'...'F',
            'a'...'f',
            '.',
            'x',
            => try buf.append(allocator, b),
            '_' => sep_chars += 1,
            else => break,
        }

        reader.toss(1);
    }

    if (std.mem.containsAtLeastScalar(u8, buf.items, 1, '.')) {
        return Token{ .real = .{ .value = try std.fmt.parseFloat(f64, buf.items), .length = buf.items.len + sep_chars } };
    } else {
        return Token{ .integer = .{ .value = try std.fmt.parseInt(i64, buf.items, 0), .length = buf.items.len + sep_chars } };
    }
}

test "tokenizeNumber" {
    const string = "1";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeNumber(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .integer = .{ .value = 1, .length = 1 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(0, remaining.len);
}

test "tokenizeNumber int" {
    const string = "4312 fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeNumber(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .integer = .{ .value = 4312, .length = 4 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(8, remaining.len);
}

test "tokenizeNumber underscore" {
    const string = "0xFFFF_FFFF fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeNumber(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .integer = .{ .value = 0xFFFF_FFFF, .length = 11 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(8, remaining.len);
}

test "tokenizeNumber overflow" {
    const string = "0xFFFFFFFFFFFFFFFFFFFF fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t = tokenizeNumber(&r, std.testing.allocator);
    try std.testing.expectError(std.fmt.ParseIntError.Overflow, t);
}

test "tokenizeNumber hex" {
    const string = "0x4312 fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeNumber(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .integer = .{ .value = 0x4312, .length = 6 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(8, remaining.len);
}

test "tokenizeNumber malformed hex" {
    const string = "40x312 fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t = tokenizeNumber(&r, std.testing.allocator);
    try std.testing.expectError(std.fmt.ParseIntError.InvalidCharacter, t);
}

test "tokenizeNumber real" {
    const string = "4312.431 fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t: Token = try tokenizeNumber(&r, std.testing.allocator);
    defer t.deinit(std.testing.allocator);
    try std.testing.expectEqualDeep(Token{ .real = .{ .value = 4312.431, .length = 8 } }, t);

    const remaining = try r.allocRemaining(std.testing.allocator, .unlimited);
    defer std.testing.allocator.free(remaining);

    try std.testing.expectEqual(8, remaining.len);
}

test "tokenizeNumber malformed hex float path" {
    const string = "40x312. fda 431";
    var r: std.Io.Reader = .fixed(string);

    const t = tokenizeNumber(&r, std.testing.allocator);
    try std.testing.expectError(std.fmt.ParseFloatError.InvalidCharacter, t);
}

// test "fuzz tokenizeNumber" {
//     const Context = struct {
//         fn testOne(context: @This(), input: []const u8) anyerror!void {
//             _ = context;
//             // Try passing `--fuzz` to `zig build test` and see if it manages to fail this test case!
//             // try std.testing.expect(!std.mem.eql(u8, "canyoufindme", input));
//             var r: std.Io.Reader = .fixed(input);
//             const t = try tokenizeNumber(&r, std.testing.allocator);
//             switch (t) {
//                 .integer => |_| {},
//                 .real => |_| {},
//                 else => unreachable,
//             }
//         }
//     };
//     try std.testing.fuzz(Context{}, Context.testOne, .{});
// }
