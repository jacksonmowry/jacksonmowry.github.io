pub const TrainingData = struct {
    data: []f16,
    label: []const f16,
};

pub fn sigmoid(z: f16) f16 {
    return 1.0 / (1.0 + @exp(-z));
}

pub fn sigmoid_prime(z: f16) f16 {
    return sigmoid(z) * (1 - sigmoid(z));
}

pub fn cost_derivative(a: f16, y: f16) f16 {
    return a - y;
}

pub const Network = struct {
    const Self = @This();
    num_layers: usize,
    sizes: []const usize,
    biases: [][]f16,
    weights: [][][]f16,
    z: [][]f16,
    activations: [][]f16,
    nabla_b: [][]f16,
    nabla_w: [][][]f16,
    delta_nabla_b: [][]f16,
    delta_nabla_w: [][][]f16,
    allocator: std.mem.Allocator,

    fn init(allocator: std.mem.Allocator, sizes: []const usize) !Self {
        var prng = std.Random.DefaultPrng.init(blk: {
            var seed: u64 = undefined;
            try std.posix.getrandom(std.mem.asBytes(&seed));
            break :blk seed;
        });
        const rand = prng.random();

        const weights = try allocator.alloc([][]f16, sizes.len - 1);
        const nabla_w = try allocator.alloc([][]f16, sizes.len - 1);
        const delta_nabla_w = try allocator.alloc([][]f16, sizes.len - 1);

        for (weights, nabla_w, delta_nabla_w, 0..) |*w, *nw, *dnw, i| {
            w.* = try allocator.alloc([]f16, sizes[i]);
            nw.* = try allocator.alloc([]f16, sizes[i]);
            dnw.* = try allocator.alloc([]f16, sizes[i]);
            for (0..sizes[i]) |n| {
                w.*[n] = try allocator.alloc(f16, sizes[i + 1]);
                nw.*[n] = try allocator.alloc(f16, sizes[i + 1]);
                dnw.*[n] = try allocator.alloc(f16, sizes[i + 1]);
                for (w.*[n]) |*fw| {
                    fw.* = @floatCast(std.Random.floatNorm(rand, f32));
                }
            }
        }

        const biases = try allocator.alloc([]f16, sizes.len);
        const nabla_b = try allocator.alloc([]f16, sizes.len);
        const delta_nabla_b = try allocator.alloc([]f16, sizes.len);
        const activations = try allocator.alloc([]f16, sizes.len);
        const z = try allocator.alloc([]f16, sizes.len);

        for (sizes, biases, nabla_b, delta_nabla_b, activations, z) |s, *b, *nb, *dnb, *sc, *zv| {
            b.* = try allocator.alloc(f16, s);
            nb.* = try allocator.alloc(f16, s);
            dnb.* = try allocator.alloc(f16, s);
            sc.* = try allocator.alloc(f16, s);
            zv.* = try allocator.alloc(f16, s);
        }

        for (activations, z) |s, zv| {
            @memset(s, 0);
            @memset(zv, 0);
        }

        for (biases) |b| {
            for (b) |*bias| {
                bias.* = @floatCast(std.Random.floatNorm(rand, f32));
            }
        }

        @memset(biases[0], 0);
        const sizes_dupe = try allocator.dupe(usize, sizes);

        return Self{
            .num_layers = sizes.len,
            .sizes = sizes_dupe,
            .biases = biases,
            .weights = weights,
            .nabla_b = nabla_b,
            .nabla_w = nabla_w,
            .delta_nabla_b = delta_nabla_b,
            .delta_nabla_w = delta_nabla_w,
            .activations = activations,
            .z = z,
            .allocator = allocator,
        };
    }

    fn feed_forward(self: Self, input: []const f16) []f16 {
        // Zero out the entire scratch matrix
        for (self.activations) |s| {
            @memset(s, 0);
        }

        @memcpy(self.activations[0], input);

        for (0..self.num_layers - 1) |i| {
            for (self.z[i + 1], self.activations[i + 1], self.biases[i + 1], 0..) |*z, *s, b, j| {
                for (self.activations[i], self.weights[i]) |sc, w| {
                    s.* += sc * w[j];
                }

                s.* += b;
                z.* = s.*;
                s.* = sigmoid(s.*);
            }
        }

        return self.activations[self.num_layers - 1];
    }

    fn sgd(
        self: Self,
        training_data: []TrainingData,
        epochs: usize,
        mini_batch_size: usize,
        eta: f16,
        test_data: ?[]TrainingData,
    ) !void {
        var prng = std.Random.DefaultPrng.init(blk: {
            var seed: u64 = undefined;
            try std.posix.getrandom(std.mem.asBytes(&seed));
            break :blk seed;
        });
        const rand = prng.random();

        var n_test: usize = 0;
        if (test_data) |td| {
            n_test = td.len;
        }

        for (0..epochs) |i| {
            std.Random.shuffle(rand, TrainingData, training_data);

            var mini_batches = std.mem.window(
                TrainingData,
                training_data,
                mini_batch_size,
                mini_batch_size,
            );

            while (mini_batches.next()) |batch| {
                self.update_mini_batch(batch, eta);
            }

            if (test_data) |td| {
                std.debug.print("Epoch {}: {} / {}\n", .{ i, self.evaluate(td), td.len });
            } else {
                std.debug.print("Epoch {} complete\n", .{i});
            }
        }
    }

    fn update_mini_batch(
        self: Self,
        mini_batch: []const TrainingData,
        eta: f16,
    ) void {
        for (self.nabla_w) |nw| {
            for (nw) |nww| {
                @memset(nww, 0);
            }
        }
        for (self.nabla_b) |nb| {
            @memset(nb, 0);
        }

        for (mini_batch) |sample| {
            self.backprop(sample.data, sample.label);

            // Add the backprop results into the nabla matricies
            for (self.nabla_b, self.delta_nabla_b) |nb, dnb| {
                for (nb, dnb) |*nbv, dnbv| {
                    nbv.* += dnbv;
                }
            }
            for (self.nabla_w, self.delta_nabla_w) |nw, dnw| {
                for (nw, dnw) |nws, dnws| {
                    for (nws, dnws) |*nwv, dnwv| {
                        nwv.* += dnwv;
                    }
                }
            }
        }

        // Update weights and biases according to nablas
        for (self.biases, self.nabla_b) |bs, nbs| {
            for (bs, nbs) |*b, nb| {
                b.* = b.* - (eta / @as(f16, @floatFromInt(mini_batch.len))) * nb;
            }
        }
        for (self.weights, self.nabla_w) |ws, nws| {
            for (ws, nws) |wss, nwss| {
                for (wss, nwss) |*w, nw| {
                    w.* = w.* - (eta / @as(f16, @floatFromInt(mini_batch.len))) * nw;
                }
            }
        }
    }

    fn backprop(self: Self, x: []f16, y: []const f16) void {
        for (self.delta_nabla_w) |nw| {
            for (nw) |nww| {
                @memset(nww, 0);
            }
        }
        for (self.delta_nabla_b) |nb| {
            @memset(nb, 0);
        }

        _ = self.feed_forward(x);

        // Backward pass

        for (
            self.delta_nabla_b[self.num_layers - 1],
            self.activations[self.num_layers - 1],
            y,
            self.z[self.num_layers - 1],
        ) |*dnb, a, yv, zv| {
            dnb.* = cost_derivative(a, yv) * sigmoid_prime(zv);
        }

        for (self.nabla_b[self.num_layers - 1], 0..) |nb, row| {
            for (self.activations[self.num_layers - 1], 0..) |a, col| {
                self.delta_nabla_w[self.num_layers - 1 - 1][row][col] = nb * a;
            }
        }

        for (2..self.num_layers) |l| {
            const layer = self.num_layers - l;

            for (self.nabla_b[layer], self.z[layer]) |*nb, z| {
                nb.* = sigmoid_prime(z);
            }

            for (0..self.weights[layer][0].len) |col| {
                var val: f16 = 0.0;
                for (self.nabla_b[layer + 1], 0..) |nw, row| {
                    val += nw * self.weights[layer][row][col];
                }

                self.nabla_b[layer][col] *= val;
            }

            for (self.nabla_b[layer], 0..) |nb, row| {
                for (self.activations[layer - 1], 0..) |a, col| {
                    self.delta_nabla_w[layer - 1][col][row] = nb * a;
                }
            }
        }
    }

    fn evaluate(self: Self, test_data: []TrainingData) usize {
        var correct: usize = 0;
        for (test_data) |td| {
            const out = self.feed_forward(td.data);
            var max: f16 = -1.0;
            var max_idx: isize = -1;
            for (out, 0..) |o, i| {
                if (o > max) {
                    max = o;
                    max_idx = @intCast(i);
                }
            }

            for (td.label, 0..) |v, i| {
                if (v == 1 and i == max_idx) {
                    correct += 1;
                    break;
                }
            }
        }

        return correct;
    }
};

const classes = [10][10]f16{
    [_]f16{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    [_]f16{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    [_]f16{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    [_]f16{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    [_]f16{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
    [_]f16{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    [_]f16{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
    [_]f16{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
    [_]f16{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    [_]f16{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
};

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();

    const args = try std.process.argsAlloc(allocator);

    const input_file = try std.fs.openFileAbsolute(args[1], .{});
    defer input_file.close();

    var training_data = std.ArrayList(TrainingData).init(allocator);
    defer training_data.deinit();

    var reader = input_file.reader();
    var buf = [_]u8{0} ** 8192;
    // Skip first line
    _ = try reader.readUntilDelimiterOrEof(&buf, '\n');
    while (try reader.readUntilDelimiterOrEof(&buf, '\n')) |read| {
        var itr = std.mem.splitAny(u8, buf[0 .. read.len - 1], ",");
        const class = try std.fmt.parseInt(usize, itr.next().?, 10);
        var pixels = std.ArrayList(f16).init(allocator);
        defer pixels.deinit();

        while (itr.next()) |val| {
            const pixel = try std.fmt.parseFloat(f16, val);
            try pixels.append(pixel / 255);
        }

        try training_data.append(TrainingData{
            .data = try pixels.toOwnedSlice(),
            .label = &classes[class],
        });
    }

    std.debug.print("I have {} training examples\n", .{training_data.items.len});

    defer {
        for (training_data.items) |*td| {
            allocator.free(td.data);
            td.* = undefined;
        }
    }

    const n = try Network.init(allocator, &[_]usize{ 784, 128, 64, 32, 10 });

    // std.debug.print("Weights: \n", .{});
    // for (n.weights) |outer_weights| {
    //     for (outer_weights) |rows| {
    //         for (rows) |val| {
    //             std.debug.print("{}, ", .{val});
    //         }
    //         std.debug.print("\n", .{});
    //     }
    //     std.debug.print("\n", .{});
    // }

    // std.debug.print("Biases: \n", .{});
    // for (n.biases) |w| {
    //     for (w) |weight| {
    //         std.debug.print("{}, ", .{weight});
    //     }
    //     std.debug.print("\n", .{});
    // }

    // const outputs = n.feed_forward(&.{ 4, 9, 6 });

    // std.debug.print("outputs: \n", .{});
    // for (outputs) |weight| {
    //     std.debug.print("{}, ", .{weight});
    // }
    // std.debug.print("\n", .{});

    try n.sgd(training_data.items, 100, 100, 3, training_data.items);
}

const std = @import("std");

// /// This imports the separate module containing `root.zig`. Take a look in `build.zig` for details.
