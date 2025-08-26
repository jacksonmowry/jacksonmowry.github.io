const std = @import("std");
const Allocator = std.mem.Allocator;

pub const Token = union(enum) {
    l_paren,
    r_paren,
    l_brace,
    r_brace,
    l_bracket,
    r_bracket,
    plus,
    minus,
    division,
    multiplication,
    le,
    lt,
    ge,
    gt,
    assignment,
    eq,
    neq,
    logical_not,
    logical_and,
    logical_or,
    bitwise_not,
    bitwise_and,
    bitwise_or,
    bitwise_xor,
    dot,
    comma,
    colon,
    semicolon,
    keyword_for,
    keyword_while,
    keyword_const,
    keyword_var,
    keyword_return,
    keyword_function,
    keyword_continue,
    keyword_break,
    keyword_defer,
    char_literal: u8,
    integer: struct { value: i64, length: usize },
    real: struct { value: f64, length: usize },
    string_literal: struct { value: []const u8, length: usize },
    template_string: struct { tokens: []Token, length: usize },
    identifier: struct { value: []const u8, length: usize },

    const Self = @This();

    pub fn deinit(self: Self, allocator: Allocator) void {
        switch (self) {
            .string_literal => |s| allocator.free(s.value),
            .identifier => |i| allocator.free(i.value),
            .template_string => |ts| {
                for (ts.tokens) |tok| {
                    tok.deinit(allocator);
                }

                allocator.free(ts.tokens);
            },
            else => return,
        }
    }

    pub fn debugPrint(self: Self) void {
        switch (self) {
            .char_literal => |c| std.debug.print("{}\n", .{c}),
            .integer => |i| std.debug.print("{}\n", .{i}),
            .real => |r| std.debug.print("{}\n", .{r}),
            .string_literal => |s| std.debug.print("{s}\n", .{s.value}),
            .identifier => |s| std.debug.print("{s}\n", .{s.value}),
            else => unreachable,
        }
    }

    pub fn tokenLen(self: Self) usize {
        return switch (self) {
            .integer,
            .real,
            .string_literal,
            .identifier,
            .template_string,
            .char_literal,
            => 0,
            else => self.fixedTokenLen(),
        };
    }

    fn fixedTokenLen(self: Self) usize {
        return switch (self) {
            .l_paren => 1,
            .r_paren => 1,
            .l_brace => 1,
            .r_brace => 1,
            .l_bracket => 1,
            .r_bracket => 1,
            .plus => 1,
            .minus => 1,
            .division => 1,
            .multiplication => 1,
            .le => 2,
            .lt => 1,
            .ge => 2,
            .gt => 1,
            .assignment => 1,
            .eq => 2,
            .neq => 2,
            .logical_not => 1,
            .logical_and => 2,
            .logical_or => 2,
            .bitwise_not => 1,
            .bitwise_and => 1,
            .bitwise_or => 1,
            .bitwise_xor => 1,
            .dot => 1,
            .comma => 1,
            .colon => 1,
            .semicolon => 1,
            .keyword_for => 3,
            .keyword_while => 5,
            .keyword_const => 5,
            .keyword_var => 3,
            .keyword_return => 6,
            .keyword_function => 8,
            .keyword_continue => 8,
            .keyword_break => 5,
            .keyword_defer => 5,
            else => 0,
        };
    }
};
