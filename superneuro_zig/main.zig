const std = @import("std");
const ArrayListUnmanaged = std.ArrayListUnmanaged;
const AutoHashMapUnmanaged = std.AutoHashMapUnmanaged;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectError = std.testing.expectError;

const NeuromorphicModel = struct {
    const Spikes = struct {
        nids: ArrayListUnmanaged(u32) = .empty,
        values: ArrayListUnmanaged(f64) = .empty,

        fn deinit(self: *Spikes, allocator: std.mem.Allocator) void {
            self.nids.deinit(allocator);
            self.values.deinit(allocator);

            self.* = undefined;
        }
    };

    // Neuron Parameters
    num_neurons: u32 = 0,
    neuron_thresholds: ArrayListUnmanaged(f64) = .empty,
    neuron_leaks: ArrayListUnmanaged(f64) = .empty,
    neuron_reset_states: ArrayListUnmanaged(f64) = .empty,
    neuron_refractory_periods: ArrayListUnmanaged(u32) = .empty,

    // Synapse Parameters
    num_synapses: u32 = 0,
    pre_synaptic_neuron_ids: ArrayListUnmanaged(u32) = .empty,
    post_synaptic_neuron_ids: ArrayListUnmanaged(u32) = .empty,
    synaptic_weights: ArrayListUnmanaged(f64) = .empty,
    synaptic_delays: ArrayListUnmanaged(u32) = .empty,
    enable_stdp: ArrayListUnmanaged(u8) = .empty,

    // Input Spikes (can have a value)
    input_spikes: AutoHashMapUnmanaged(u32, Spikes) = .empty,

    // Spike Trains (monitoring all neurons)

    // STDP Parameters
    stdp: bool = false,
    stdp_time_steps: usize = 0,
    stdp_Apos: []f64 = &.{},
    stdp_Aneg: []f64 = &.{},
    stdp_positive_update: bool = false,
    stdp_negative_update: bool = false,

    // Simulation State
    weight_matrix: NxNMatrixUnmanaged(f64) = .empty,
    stdp_matrix: NxNMatrixUnmanaged(u8) = .empty,
    internal_state: NVectorUnmanaged(f64) = .empty,
    internal_input_spikes: NVectorUnmanaged(f64) = .empty,
    internal_spikes: NVectorUnmanaged(f64) = .empty,

    // Internal allocator
    allocator: std.mem.Allocator,

    fn init(allocator: std.mem.Allocator) !NeuromorphicModel {
        return NeuromorphicModel{
            .allocator = allocator,
        };
    }

    fn deinit(self: *NeuromorphicModel) void {
        var itr = self.input_spikes.iterator();
        while (itr.next()) |entry| {
            entry.value_ptr.deinit(self.allocator);
        }

        self.neuron_thresholds.deinit(self.allocator);
        self.neuron_leaks.deinit(self.allocator);
        self.neuron_reset_states.deinit(self.allocator);
        self.neuron_refractory_periods.deinit(self.allocator);
        self.pre_synaptic_neuron_ids.deinit(self.allocator);
        self.post_synaptic_neuron_ids.deinit(self.allocator);
        self.synaptic_weights.deinit(self.allocator);
        self.synaptic_delays.deinit(self.allocator);
        self.enable_stdp.deinit(self.allocator);
        self.input_spikes.deinit(self.allocator);

        if (self.stdp_Apos.len != 0) {
            self.allocator.free(self.stdp_Apos);
        }
        if (self.stdp_Aneg.len != 0) {
            self.allocator.free(self.stdp_Aneg);
        }

        if (self.weight_matrix.dim != 0) {
            self.weight_matrix.deinit(self.allocator);
        }
        if (self.stdp_matrix.dim != 0) {
            self.stdp_matrix.deinit(self.allocator);
        }
        if (self.internal_state.len != 0) {
            self.internal_state.deinit(self.allocator);
        }
        if (self.internal_input_spikes.len != 0) {
            self.internal_input_spikes.deinit(self.allocator);
        }
        if (self.internal_spikes.len != 0) {
            self.internal_spikes.deinit(self.allocator);
        }

        self.* = undefined;
    }

    fn create_neuron(
        self: *NeuromorphicModel,
        threshold: f64,
        leak: f64,
        reset_state: f64,
        refractory_period: u32,
    ) !u32 {
        if (leak < 0.0) {
            return error.LeakError;
        }

        try self.neuron_thresholds.append(self.allocator, threshold);
        try self.neuron_leaks.append(self.allocator, leak);
        try self.neuron_reset_states.append(self.allocator, reset_state);
        try self.neuron_refractory_periods.append(self.allocator, refractory_period);

        self.num_neurons += 1;

        return self.num_neurons - 1;
    }

    fn create_synapse(
        self: *NeuromorphicModel,
        pre_id: u32,
        post_id: u32,
        weight: f64,
        delay: u32,
        stdp_enabled: bool,
    ) !u32 {
        if (delay == 0) {
            return error.DelayZero;
        }

        if (delay == 1) {
            try self.pre_synaptic_neuron_ids.append(self.allocator, pre_id);
            try self.post_synaptic_neuron_ids.append(self.allocator, post_id);
            try self.synaptic_weights.append(self.allocator, weight);
            try self.synaptic_delays.append(self.allocator, delay);
            try self.enable_stdp.append(self.allocator, @intFromBool(stdp_enabled));

            self.num_synapses += 1;
        } else {
            // Zig doesn't allow mutating function parameters
            var tracking_pre_id = pre_id;
            for (0..delay) |_| {
                const temp_id = try self.create_neuron(0.0, std.math.inf(f64), 0.0, 0);
                _ = try self.create_synapse(tracking_pre_id, temp_id, 1.0, 1, false);

                tracking_pre_id = temp_id;
            }

            _ = try self.create_synapse(tracking_pre_id, post_id, weight, delay, stdp_enabled);
        }

        return self.num_synapses - 1;
    }

    fn add_spike(
        self: *NeuromorphicModel,
        time: u32,
        neuron_id: u32,
        value: f64,
    ) !void {
        if (self.input_spikes.getPtr(time)) |spikes| {
            try spikes.nids.append(self.allocator, neuron_id);
            try spikes.values.append(self.allocator, value);
        } else {
            try self.input_spikes.put(self.allocator, time, .{});
            const spikes = self.input_spikes.getPtr(time) orelse unreachable;

            try spikes.nids.append(self.allocator, neuron_id);
            try spikes.values.append(self.allocator, value);
        }
    }

    fn stdp_setup(
        self: *NeuromorphicModel,
        time_steps: u32,
        Apos: []f64,
        Aneg: []f64,
        positive_update: bool,
        negative_update: bool,
    ) !void {
        if (time_steps == 0) {
            return error.TimeStepsZero;
        }

        if (time_steps != Apos.len) {
            return error.AposLenNotEqual;
        }

        if (time_steps != Aneg.len) {
            return error.AnegLenNotEqual;
        }

        if (Apos.len != Aneg.len) {
            return error.AnegLenNotEqualAposLen;
        }

        for (Apos, Aneg) |pos, neg| {
            if (pos < 0) {
                return error.AposElemLTZero;
            }

            if (neg < 0) {
                return error.AnegElemLTZero;
            }
        }

        for (self.enable_stdp) |enabled| {
            if (enabled == true) {
                self.stdp = true;
                self.stdp_time_steps = time_steps;
                self.stdp_Apos = self.allocator.dupe([]f64, Apos);
                self.stdp_Aneg = self.allocator.dupe([]f64, Aneg);
                self.stdp_positive_update = positive_update;
                self.stdp_negative_update = negative_update;

                break;
            }
        } else {
            // No synapses have stdp enabled, don't need to call this method
            return error.NoSTDPEnabled;
        }
    }

    fn setup(self: *NeuromorphicModel) !void {
        // We need a 2D array of size (num_neurons * num_neurons), called `weights`
        // Same thing for stdp enabled synapses
        // Lastly an array just for input spikes, sizeof(num_neurons)

        // Ensure the weight matrix is not initialized, otherwise free and reallocate
        if (self.weight_matrix.dim != 0) {
            self.weight_matrix.deinit(self.allocator);
        }
        if (self.stdp_matrix.dim != 0) {
            self.stdp_matrix.deinit(self.allocator);
        }

        self.weight_matrix = try NxNMatrixUnmanaged(f64).init(
            self.allocator,
            self.num_neurons,
        );
        self.stdp_matrix = try NxNMatrixUnmanaged(u8).init(
            self.allocator,
            self.num_neurons,
        );
        self.internal_state = try NVectorUnmanaged(f64).init(
            self.allocator,
            self.num_neurons,
        );
        self.internal_input_spikes = try NVectorUnmanaged(f64).init(
            self.allocator,
            self.num_neurons,
        );

        try self.weight_matrix.assign2D(
            self.pre_synaptic_neuron_ids.items,
            self.post_synaptic_neuron_ids.items,
            self.synaptic_weights.items,
        );

        if (self.stdp == true) {
            try self.stdp_matrix.assign2D(
                self.pre_synaptic_neuron_ids.items,
                self.post_synaptic_neuron_ids.items,
                self.enable_stdp.items,
            );
        }
    }

    fn simulate(
        self: *NeuromorphicModel,
        time_steps: usize,
    ) !void {
        if (time_steps == 0) {
            return error.TimeStepsEqZero;
        }

        for (0..time_steps) |time_step| {
            // Leak: internal state > reset state
            for (
                self.internal_state.vec,
                self.neuron_reset_states.items,
                self.neuron_leaks.items,
            ) |*state, reset_state, leak| {
                if (state.* > reset_state) {
                    state.* = @max(state.* - leak, reset_state);
                } else if (state.* < reset_state) {
                    state.* = @min(state.* + leak, reset_state);
                }
            }

            // Zero out input spikes to prepare them for input spikes in current time step
            self.internal_input_spikes.clear();

            // Apply Input Spikes if we have any
            if (self.input_spikes.get(time_step)) |spikes| {
                for (spikes.nids, spikes.values) |nid, value| {
                    self.internal_input_spikes[nid] = value;
                }
            }

            // Internal State
            self.internal_state.add(self.internal_input_spikes);
        }
    }
};

test "Create Neurons" {
    var nm = try NeuromorphicModel.init(std.testing.allocator);
    defer nm.deinit();

    try expectEqual(0, try nm.create_neuron(1, 1, 0, 0));
    try expectEqual(1, try nm.create_neuron(1, 1, 0, 0));
}

test "Create Synapses" {
    var nm = try NeuromorphicModel.init(std.testing.allocator);
    defer nm.deinit();

    try std.testing.expectError(error.DelayZero, nm.create_synapse(0, 0, 0, 0, false));
    try expectEqual(0, try nm.create_synapse(0, 0, 1, 1, false));
    try expectEqual(1, try nm.create_synapse(0, 0, 1, 1, false));
}

test "Add Spikes" {
    var nm = try NeuromorphicModel.init(std.testing.allocator);
    defer nm.deinit();

    try nm.add_spike(0, 0, 1);
}

test "Call Setup" {
    var nm = try NeuromorphicModel.init(std.testing.allocator);
    defer nm.deinit();

    _ = try nm.create_neuron(1, 1, 0, 0);
    _ = try nm.create_neuron(1, 1, 0, 0);
    _ = try nm.create_synapse(0, 1, 1, 1, true);
    _ = try nm.create_synapse(1, 0, 1, 1, false);

    try nm.setup();
}

pub fn NVectorUnmanaged(T: type) type {
    return struct {
        const Self = @This();
        len: u32,
        vec: []T,

        pub const empty: Self = .{
            .len = 0,
            .vec = &.{},
        };

        fn init(allocator: std.mem.Allocator, size: u32) !Self {
            const s = Self{
                .len = size,
                .vec = try allocator.alloc(T, size),
            };

            @memset(s.vec, @as(T, 0));

            return s;
        }

        fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            allocator.free(self.vec);

            self.* = undefined;
        }

        fn clear(self: *Self) void {
            @memset(self.vec, 0);
        }

        fn get(self: Self, elem: u32) !T {
            if (elem >= self.len) {
                return error.OutOfBounds;
            }

            return self.vec[elem];
        }

        fn set(self: *Self, elem: u32, val: T) !void {
            if (elem >= self.len) {
                return error.OutOfBounds;
            }

            self.vec[elem] = val;
        }

        fn add(self: *Self, other: Self) !void {
            if (self.len != other.len) {
                return error.LenNEq;
            }

            for (self.vec, other.vec) |*s, o| {
                s.* += o;
            }
        }

        /// Modifies the original vector in place
        fn matrixMultiply(self: *Self, matrix: NxNMatrixUnmanaged(T)) !void {}
    };
}

test "Make vec" {
    var vec = try NVectorUnmanaged(f64).init(std.testing.allocator, 42);
    defer vec.deinit(std.testing.allocator);

    for (0..42) |elem| {
        try expectEqual(0, try vec.get(@intCast(elem)));
    }

    try expectEqual(42, vec.len);
}

test "Vector Set" {
    var vector = try NVectorUnmanaged(f64).init(std.testing.allocator, 42);
    defer vector.deinit(std.testing.allocator);

    try vector.set(4, 56);
    try expectEqual(56, try vector.get(4));
}

test "Vector add" {
    var vector = try NVectorUnmanaged(f64).init(std.testing.allocator, 2);
    defer vector.deinit(std.testing.allocator);
    try vector.set(0, 1);
    try vector.set(1, 2);

    var vector2 = try NVectorUnmanaged(f64).init(std.testing.allocator, 2);
    defer vector2.deinit(std.testing.allocator);
    try vector2.set(0, 1);
    try vector2.set(1, 2);

    try vector.add(vector2);

    try expect(std.mem.eql(f64, vector.vec, &.{ 2, 4 }));
}

pub fn NxNMatrixUnmanaged(T: type) type {
    return struct {
        const Self = @This();
        dim: u32,
        mat: []T,

        pub const empty: Self = .{
            .dim = 0,
            .mat = &.{},
        };

        fn init(allocator: std.mem.Allocator, size: u32) !Self {
            const s = Self{
                .dim = size,
                .mat = try allocator.alloc(T, size * size),
            };

            @memset(s.mat, @as(T, 0));

            return s;
        }

        fn deinit(self: *Self, allocator: std.mem.Allocator) void {
            allocator.free(self.mat);

            self.* = undefined;
        }

        fn get(self: Self, row: u32, col: u32) !T {
            if (row >= self.dim or col >= self.dim) {
                return error.OutOfBounds;
            }

            return self.mat[(row * self.dim) + col];
        }

        fn set(self: *Self, row: u32, col: u32, val: T) !void {
            if (row >= self.dim or col >= self.dim) {
                return error.OutOfBounds;
            }

            self.mat[(row * self.dim) + col] = val;
        }

        fn assign2D(self: *Self, rows: []const u32, cols: []const u32, vals: []const T) !void {
            for (rows, cols, vals) |r, c, v| {
                try self.set(r, c, v);
            }
        }
    };
}

test "Make mat" {
    var matrix = try NxNMatrixUnmanaged(f64).init(std.testing.allocator, 42);
    defer matrix.deinit(std.testing.allocator);

    for (0..42) |row| {
        for (0..42) |col| {
            try expectEqual(0, try matrix.get(@intCast(row), @intCast(col)));
        }
    }

    try expectEqual(42, matrix.dim);
}

test "Set" {
    var matrix = try NxNMatrixUnmanaged(f64).init(std.testing.allocator, 42);
    defer matrix.deinit(std.testing.allocator);

    try matrix.set(4, 4, 56);
    try expectEqual(56, try matrix.get(4, 4));
}

test "assign2D" {
    var matrix = try NxNMatrixUnmanaged(f64).init(std.testing.allocator, 4);
    defer matrix.deinit(std.testing.allocator);

    try matrix.assign2D(&.{ 0, 1, 2, 3 }, &.{ 3, 2, 1, 0 }, &.{ 7, 8, 9, 10 });

    try expect(std.mem.eql(f64, matrix.mat, &.{ 0, 0, 0, 7, 0, 0, 8, 0, 0, 9, 0, 0, 10, 0, 0, 0 }));
    // array([[ 0.,  0.,  0.,  7.],
    //        [ 0.,  0.,  8.,  0.],
    //        [ 0.,  9.,  0.,  0.],
    //        [10.,  0.,  0.,  0.]])
}
