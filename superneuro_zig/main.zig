const std = @import("std");
const ArrayListUnmanaged = std.ArrayListUnmanaged;

const NeuromorphicModel = struct {
    // Neuron Parameters
    num_neurons: usize = 0,
    neuron_thresholds: ArrayListUnmanaged(f64),
    neuron_leaks: ArrayListUnmanaged(f64),
    neuron_reset_states: ArrayListUnmanaged(f64),
    neuron_refractory_periods: ArrayListUnmanaged(f64),

    // Synapse Parameters
    num_synapses: usize = 0,
    pre_synaptic_neuron_ids: ArrayListUnmanaged(i32),
    post_synaptic_neuron_ids: ArrayListUnmanaged(i32),
    synaptic_weights: ArrayListUnmanaged(f64),
    synaptic_delays: ArrayListUnmanaged(i32),
    enable_stdp: ArrayListUnmanaged(bool),

    // Input Spikes (can have a value)
    input_spikes: void = undefined,

    // Spike Trains (monitoring all neurons)

    // STDP Parameters
    stdp: bool = false,
    stdp_time_steps: usize = 0,
    stdp_Apos: ArrayListUnmanaged(f64),
    stdp_Aneg: ArrayListUnmanaged(f64),
    stdp_positive_update: bool = false,
    stdp_negative_update: bool = false,

    // Internal allocator
    allocator: std.mem.Allocator,

    fn init(allocator: std.mem.Allocator) !NeuromorphicModel {
        return NeuromorphicModel{
            .allocator = allocator,
        };
    }

    fn create_neuron(
        self: *NeuromorphicModel,
        threshold: f64,
        leak: f64,
        reset_state: f64,
        refractory_period: i32,
    ) error{ LeakError, Refractory_Error }!i32 {
        if (leak < 0.0) {
            return error.ValueError;
        }

        if (refractory_period < 0) {
            return error.Refractory_Error;
        }

        try self.neuron_thresholds.append(self.allocator, threshold);
        try self.neuron_leaks.append(self.allocator, leak);
        try self.neuron_reset_states.append(self.allocator, reset_state);
        try self.neuron_refractory_periods.append(self.allocator, refractory_period);

        self.num_neurons += 1;

        return self.num_neurons - 1;
    }

    fn create_synapse(
        self: NeuromorphicModel,
        pre_id: u32,
        post_id: u32,
        weight: f64,
        delay: u32,
        stdp_enabled: bool,
    ) !i32 {
        if (delay == 1) {
            try self.pre_synaptic_neuron_ids.append(self.allocator, pre_id);
            try self.post_synaptic_neuron_ids.append(self.allocator, post_id);
            try self.synaptic_weights.append(self.allocator, weight);
            try self.synaptic_delays.append(self.allocator, delay);
            try self.enable_stdp.append(self.allocator, stdp_enabled);

            self.num_synapses += 1;
        } else {
            // Zig doesn't allow mutating function parameters
            var tracking_pre_id = pre_id;
            for (0..delay) |_| {
                const temp_id = self.create_neuron(0.0, std.math.inf(f64), 0.0, 0);
                self.create_synapse(tracking_pre_id, temp_id, 1.0, 1, false);

                tracking_pre_id = temp_id;
            }

            self.create_synapse(tracking_pre_id, post_id, weight, stdp_enabled);
        }

        return self.num_synapses - 1;
    }

    fn add_spike(
        self: NeuromorphicModel,
        time: u32,
        neuron_id: u32,
        value: f64,
    ) !void {}
};

pub fn main() !void {
    var alloc = std.heap.GeneralPurposeAllocator(.{}){};
    const gpa = alloc.allocator();
    const nm = try NeuromorphicModel.init(gpa, false);
    std.debug.print("{}\n", .{nm});
}
