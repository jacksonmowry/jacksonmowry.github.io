const std = @import("std");
const file = @embedFile(".part1.txt");
pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        _ = gpa.deinit();
    }
    const allocator = gpa.allocator();

    var left_list = std.ArrayList(i32).init(allocator);
    defer left_list.deinit();

    var right_map = std.AutoHashMap(i32, i32).init(allocator);
    defer right_map.deinit();

    var iter = std.mem.tokenizeAny(u8, file, " \n");

    var left = true;
    while (iter.next()) |num| {
        const parsed_num = try std.fmt.parseInt(i32, num, 10);

        if (left) {
            try left_list.append(parsed_num);
        } else {
            const val = right_map.get(parsed_num);
            if (val) |value| {
                try right_map.put(parsed_num, value + 1);
            } else {
                try right_map.put(parsed_num, 1);
            }
        }

        left = !left;
    }

    // std.debug.print("{any}\n", .{left_list.items});
    // std.debug.print(
    //     "Counts, 4:{}, 3:{}, 5:{}, 9:{}\n",
    //     .{
    //         right_map.get(4).?,
    //         right_map.get(3).?,
    //         right_map.get(5).?,
    //         right_map.get(9).?,
    //     },
    // );

    // TODO update this for part 2
    var accumulator: i32 = 0;

    for (left_list.items) |l| {
        const count = right_map.get(l);

        if (count) |left_val| {
            accumulator += l * left_val;
        }
    }

    std.debug.print("answer: {}\n", .{accumulator});
}
