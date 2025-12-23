const std = @import("std");

const file = @embedFile(".input.txt");
const nodes = 26 * 26 * 26;

const Node = struct {
    edges: [24]u32,
    num_edges: u32,
};

fn idx(name: []const u8) u32 {
    const one: u32 = name[0] - 'a';
    const two: u32 = name[1] - 'a';
    const thr: u32 = name[2] - 'a';
    return (one * 26 * 26) + (two * 26) + thr;
}

fn dfs(graph: []Node, current: u32, end: u32, cache: []usize) usize {
    if (current == end) {
        return 1;
    }

    if (cache[current] != std.math.maxInt(usize)) {
        return cache[current];
    }

    var count: usize = 0;
    for (graph[current].edges[0..graph[current].num_edges]) |n| {
        count += dfs(graph, n, end, cache);
    }

    cache[current] = count;
    return count;
}

pub fn main() !void {
    var graph: [nodes]Node = undefined;

    var file_iter = std.mem.tokenizeScalar(u8, file, '\n');
    while (file_iter.next()) |line| {
        var line_iter = std.mem.tokenizeAny(u8, line, ": ");
        const node_index = idx(line_iter.next().?);

        graph[node_index].num_edges = 0;
        while (line_iter.next()) |edge| {
            const edge_index = idx(edge);
            graph[node_index].edges[graph[node_index].num_edges] = edge_index;
            graph[node_index].num_edges += 1;
        }
    }

    var cache = [_]usize{std.math.maxInt(usize)} ** nodes;
    const answer = dfs(&graph, idx("you"), idx("out"), &cache);
    std.log.debug("Answer {}", .{answer});
}
