const std = @import("std");
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;

const Allocator = std.mem.Allocator;

const Status = enum(u2) {
    empty,
    taken,
    tombstone,
};

pub fn HashmapUnmanaged(
    comptime K: type,
    comptime V: type,
    comptime hash_fn: fn (key: K) usize,
    comptime eq_fn: fn (key1: K, key2: K) bool,
) type {
    return struct {
        const Self = @This();
        const init_size = 7;
        const target_load = 50;
        const hf = hash_fn;
        const key_eq = eq_fn;

        status: []Status,
        keys: []K,
        vals: []V,
        size: usize,
        cap: usize,

        pub const KV = struct {
            key: K,
            value: V,
        };

        pub const empty: Self = .{
            .status = undefined,
            .keys = undefined,
            .vals = undefined,
            .size = 0,
            .cap = 0,
        };

        pub fn deinit(self: *Self, allocator: Allocator) void {
            allocator.free(self.status);
            allocator.free(self.keys);
            allocator.free(self.vals);

            self.* = undefined;
        }

        pub fn put(self: *Self, allocator: Allocator, key: K, val: V) !void {
            if (self.cap == 0) {
                // Special case init call
                try self.init(allocator);
            }

            if (((self.size + 1) * 100) / (self.cap) > target_load) {
                const new_cap: usize = prime: for (self.cap + 1..10000) |itr| {
                    for (2..std.math.sqrt(itr) + 1) |t| {
                        if (itr % t == 0) {
                            continue :prime;
                        }
                    } else {
                        break :prime itr;
                    }
                } else {
                    unreachable;
                };

                const new_status = try allocator.alloc(Status, new_cap);
                const new_keys = try allocator.alloc(K, new_cap);
                const new_vals = try allocator.alloc(V, new_cap);
                @memset(new_status, .empty);

                outer: for (self.status, self.keys, self.vals) |s, k, v| {
                    switch (s) {
                        .taken => {
                            const hash_index = hf(k);
                            for (0..new_cap) |offset| {
                                const probe_index = (hash_index + offset) % new_cap;
                                switch (new_status[probe_index]) {
                                    .empty => |*status| {
                                        status.* = .taken;
                                        new_keys[probe_index] = k;
                                        new_vals[probe_index] = v;

                                        continue :outer;
                                    },
                                    else => continue,
                                }
                            } else {
                                unreachable;
                            }
                        },
                        else => continue,
                    }
                }

                allocator.free(self.status);
                allocator.free(self.keys);
                allocator.free(self.vals);

                self.status = new_status;
                self.keys = new_keys;
                self.vals = new_vals;
                self.cap = new_cap;
            }

            const new_index = hf(key) % self.cap;
            var first_free: ?usize = null;

            // Simple linear probe loop
            for (0..self.cap) |offset| {
                const probe_index = (new_index + offset) % self.cap;
                switch (self.status[probe_index]) {
                    .empty => |*status| {
                        status.* = .taken;
                        self.keys[probe_index] = key;
                        self.vals[probe_index] = val;
                        self.size += 1;

                        break;
                    },
                    .taken => {
                        if (key_eq(key, self.keys[probe_index])) {
                            self.vals[probe_index] = val;

                            break;
                        }
                    },
                    .tombstone => {
                        if (first_free == null) {
                            first_free = probe_index;
                        }
                    },
                }
            } else {
                // If first_free is set that means we ran through a bunch of
                // tombstones but never found a free slot, so we can take a tombstone
                if (first_free) |ff| {
                    self.status[ff] = .taken;
                    self.keys[ff] = key;
                    self.vals[ff] = val;

                    self.size += 1;
                } else {
                    unreachable; // for now
                }
            }
        }

        pub fn get(self: Self, key: K) ?V {
            const hash_index = hf(key);

            for (0..self.cap) |offset| {
                const probe_index = (hash_index + offset) % self.cap;
                switch (self.status[probe_index]) {
                    .empty => return null,
                    .taken => {
                        if (key_eq(key, self.keys[probe_index])) {
                            return self.vals[probe_index];
                        }
                    },
                    .tombstone => {
                        continue;
                    },
                }
            } else {
                return null;
            }
        }

        pub fn remove(self: *Self, key: K) ?V {
            const hash_index = hf(key);

            for (0..self.cap) |offset| {
                const probe_index = (hash_index + offset) % self.cap;

                switch (self.status[probe_index]) {
                    .taken => |*status| {
                        if (key_eq(key, self.keys[probe_index])) {
                            status.* = .tombstone;

                            return self.vals[probe_index];
                        }
                    },
                    else => continue,
                }
            } else {
                return null;
            }
        }

        fn init(self: *Self, allocator: Allocator) !void {
            self.status = try allocator.alloc(Status, init_size);
            self.keys = try allocator.alloc(K, init_size);
            self.vals = try allocator.alloc(V, init_size);

            @memset(self.status, Status.empty);
            @memset(self.keys, undefined);
            @memset(self.vals, undefined);

            self.size = 0;
            self.cap = init_size;
        }
    };
}

fn hash(key: []const u8) usize {
    var digest: usize = 0;

    for (key, 0..) |b, i| {
        digest ^= (b << @intCast((i % 8)));
    }

    return digest;
}

fn eq(key1: []const u8, key2: []const u8) bool {
    return std.mem.eql(u8, key1, key2);
}

test "Insert 1 value" {
    var hashmap = HashmapUnmanaged([]const u8, i32, hash, eq).empty;

    const k = "this is a test key";
    const v: i32 = 4;

    try hashmap.put(std.testing.allocator, k, v);
    hashmap.deinit(std.testing.allocator);
}

fn i32hash(key: i32) usize {
    return @abs(key);
}
fn i32eq(key1: i32, key2: i32) bool {
    return key1 == key2;
}

test "Insert 2 values" {
    var hashmap = HashmapUnmanaged(i32, i32, i32hash, i32eq).empty;

    try hashmap.put(std.testing.allocator, 4, 5);
    try hashmap.put(std.testing.allocator, 4 + 7, 6);

    try expect(hashmap.get(4).? == 5);
    try expect(hashmap.get(4 + 7).? == 6);

    hashmap.deinit(std.testing.allocator);
}

test "Force Resize" {
    var hashmap = HashmapUnmanaged(i32, i32, i32hash, i32eq).empty;

    try hashmap.put(std.testing.allocator, 1, 1);
    try hashmap.put(std.testing.allocator, 2, 2);
    try hashmap.put(std.testing.allocator, 3, 3);
    try hashmap.put(std.testing.allocator, 4, 4);

    try expectEqual(11, hashmap.cap);

    try expect(hashmap.get(1).? == 1);
    try expect(hashmap.get(2).? == 2);
    try expect(hashmap.get(3).? == 3);
    try expect(hashmap.get(4).? == 4);

    hashmap.deinit(std.testing.allocator);
}

test "100" {
    var hashmap = HashmapUnmanaged(i32, i32, i32hash, i32eq).empty;

    for (0..100) |i| {
        try hashmap.put(std.testing.allocator, @intCast(i), @intCast(i));
    }

    try expectEqual(197, hashmap.cap);

    for (0..100) |i| {
        const small: i32 = @intCast(i);
        try expectEqual(small, hashmap.get(@intCast(i)).?);
    }

    hashmap.deinit(std.testing.allocator);
}

test "Removal" {
    var hashmap = HashmapUnmanaged(i32, i32, i32hash, i32eq).empty;

    try hashmap.put(std.testing.allocator, 1, 1);
    try hashmap.put(std.testing.allocator, 2, 2);
    try hashmap.put(std.testing.allocator, 3, 3);

    try expectEqual(2, hashmap.remove(2).?);
    try expectEqual(null, hashmap.remove(2));

    try expectEqual(1, hashmap.get(1).?);
    try expectEqual(3, hashmap.get(3).?);

    std.debug.print("{any}\n", .{hashmap.status});
    std.debug.print("{any}\n", .{hashmap.keys});

    hashmap.deinit(std.testing.allocator);
}
