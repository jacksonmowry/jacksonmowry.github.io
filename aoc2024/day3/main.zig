const std = @import("std");
const file = @embedFile(".part1.txt");

const Token = union(enum) {
    mul: void,
    do: void,
    dont: void,
    num: i32,
    lparen: void,
    rparen: void,
    comma: void,
    junk: void,
};

const Tokenizer = struct {
    buffer: std.BoundedArray(u8, 3),
    current_token: ?Token,
    backing_file: []const u8,
    pos: u32,

    fn next(self: *Tokenizer) ?Token {
        if (self.pos >= self.backing_file.len) {
            return null;
        }

        self.current_token = null;

        while (self.current_token == null) {
            switch (self.backing_file[self.pos]) {
                'm' => {
                    if (self.pos < self.backing_file.len - 2 and self.backing_file[self.pos + 1] == 'u' and self.backing_file[self.pos + 2] == 'l') {
                        self.current_token = .{ .mul = {} };
                        self.pos += 3;
                    } else {
                        self.pos += 1;
                    }
                },
                'd' => {
                    if (self.backing_file[self.pos + 1] == 'o' and self.backing_file[self.pos + 2] == 'n' and self.backing_file[self.pos + 3] == '\'' and self.backing_file[self.pos + 4] == 't') {
                        self.current_token = .{ .dont = {} };
                        self.pos += 5;
                    } else if (self.backing_file[self.pos + 1] == 'o') {
                        self.current_token = .{ .do = {} };
                        self.pos += 2;
                    } else {
                        self.pos += 1;
                    }
                },
                '0'...'9' => {
                    var offset: u32 = 0;
                    self.buffer.clear();
                    while (std.ascii.isDigit(self.backing_file[self.pos + offset])) {
                        self.buffer.append(self.backing_file[self.pos + offset]) catch {};
                        offset += 1;
                    }

                    var tmp_num: i32 = 0;

                    for (self.buffer.slice()) |i| {
                        tmp_num *= 10;
                        tmp_num += i - '0';
                    }

                    self.current_token = .{ .num = tmp_num };
                    self.pos += offset;
                },
                ',' => {
                    self.current_token = .{ .comma = {} };
                    self.pos += 1;
                },
                '(' => {
                    self.current_token = .{ .lparen = {} };
                    self.pos += 1;
                },
                ')' => {
                    self.current_token = .{ .rparen = {} };
                    self.pos += 1;
                },
                else => {
                    self.current_token = .{ .junk = {} };
                    self.pos += 1;
                },
            }
        }

        return self.current_token;
    }
};

pub fn main() !void {
    var t = Tokenizer{
        .buffer = try std.BoundedArray(u8, 3).init(3),
        .current_token = null,
        .backing_file = file,
        .pos = 0,
    };

    var acc: i32 = 0;
    var mul_enabled = true;
    var fa = try std.BoundedArray(Token, 6).init(6);

    while (t.next()) |tok| {
        if (tok == .junk or tok == .do or tok == .dont) {
            fa.clear();
        }

        if (tok == .do) {
            mul_enabled = true;
            continue;
        }

        if (tok == .dont) {
            mul_enabled = false;
            continue;
        }

        if (!mul_enabled) {
            continue;
        }

        switch (tok) {
            .mul => |_| {
                if (fa.len == 0) {
                    try fa.append(tok);
                } else {
                    fa.clear();
                }
            },
            .lparen => |_| {
                if (fa.len == 1) {
                    try fa.append(tok);
                } else {
                    fa.clear();
                }
            },
            .num => |_| {
                if (fa.len == 2 or fa.len == 4) {
                    try fa.append(tok);
                } else {
                    fa.clear();
                }
            },
            .comma => |_| {
                if (fa.len == 3) {
                    try fa.append(tok);
                } else {
                    fa.clear();
                }
            },
            .rparen => |_| {
                if (fa.len == 5) {
                    // We made it!
                    const val1 = switch (fa.get(2)) {
                        .num => |n| n,
                        else => unreachable,
                    };
                    const val2 = switch (fa.get(4)) {
                        .num => |n| n,
                        else => unreachable,
                    };

                    acc += val1 * val2;

                    fa.clear();
                } else {
                    fa.clear();
                }
            },
            else => |_| continue,
        }
    }

    std.debug.print("{}\n", .{acc});
}
