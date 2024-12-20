//! AOC day 11 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    stones: *std.AutoHashMap(u64, u64),
) !void {
    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return;
    };
    defer file.close();

    const delimiter = ' ';
    const max_size = std.math.maxInt(usize);
    while (file.reader().readUntilDelimiterOrEofAlloc(allocator, delimiter, max_size) catch |err| {
        std.log.err("Failed to read line: {s}", .{@errorName(err)});
        return;
    }) |word| {
        defer allocator.free(word);
        if (word.len == 0) {
            break; // skip empty lines
        }

        const num = try std.fmt.parseInt(u64, word, 10);
        if (stones.contains(num)) {
            const stone_count = stones.get(num).?;
            try stones.put(num, stone_count + 1);
        } else {
            try stones.put(num, 1);
        }
    }
}

pub fn print_stones(stones: std.AutoHashMap(u64, u32)) void {
    var stones_iterator = stones.iterator();
    while (stones_iterator.next()) |stone_entry| {
        for (0..stone_entry.value_ptr.*) |i| {
            _ = i;
            std.debug.print("{d}, ", .{stone_entry.key_ptr.*});
        }
    }
    std.debug.print("\n", .{});
}

pub fn count_digits(num: u64) u32 {
    if (num <= 9) {
        return 1;
    }

    var result: u32 = 0;
    var temp: u64 = num;
    while (temp != 0) {
        result += 1;
        temp = temp / 10;
    }
    return result;
}

pub fn split_num(num: u64) [2]u64 {
    const digits_count = count_digits(num);
    const left = num / (std.math.pow(u64, 10, (digits_count / 2)));
    const right = num % (std.math.pow(u64, 10, (digits_count / 2)));
    return [2]u64{ left, right };
}

/// rules
/// if 0: replace with 1
/// if even num of digits: replace with 2, split (1000 = 10 and 00)
/// if none of the above: stone is replace with num = num * 2024
pub fn blink(
    allocator: std.mem.Allocator,
    stones: *std.AutoHashMap(u64, u64),
) !usize {
    var out_stones = std.AutoHashMap(u64, u64).init(allocator);
    defer out_stones.deinit();

    var stones_iterator = stones.iterator();
    while (stones_iterator.next()) |stone_entry| {
        const stone = stone_entry.key_ptr.*;
        const stone_count = stone_entry.value_ptr.*;
        _ = stones.remove(stone);

        const digit_count: u32 = count_digits(stone);
        if (stone == 0) {
            if (out_stones.contains(1)) {
                try out_stones.put(1, out_stones.get(1).? + stone_count);
            } else {
                try out_stones.put(1, stone_count);
            }
        } else if (digit_count % 2 == 0) {
            const split: [2]u64 = split_num(stone);
            if (out_stones.contains(split[0])) {
                try out_stones.put(split[0], out_stones.get(split[0]).? + stone_count);
            } else {
                try out_stones.put(split[0], stone_count);
            }

            if (out_stones.contains(split[1])) {
                try out_stones.put(split[1], out_stones.get(split[1]).? + stone_count);
            } else {
                try out_stones.put(split[1], stone_count);
            }
        } else {
            const big_num = stone * 2024;
            if (out_stones.contains(big_num)) {
                try out_stones.put(big_num, out_stones.get(big_num).? + stone_count);
            } else {
                try out_stones.put(big_num, stone_count);
            }
        }
    }

    var stone_count: usize = 0;
    var out_stones_iterator = out_stones.iterator();
    while (out_stones_iterator.next()) |stone_entry| {
        try stones.put(stone_entry.key_ptr.*, stone_entry.value_ptr.*);
        stone_count += stone_entry.value_ptr.*;
    }
    return stone_count;
}

pub fn run(
    allocator: std.mem.Allocator,
    stones: *std.AutoHashMap(u64, u64),
    steps: u32,
) !u64 {
    var stone_count: usize = 0;
    for (0..steps) |i| {
        _ = i;
        stone_count = try blink(allocator, stones);
        //std.debug.print("{d}: {d}\n", .{ i + 1, stone_count });
        //print_stones(stones.*);
    }
    return stone_count;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len < 2) {
        std.log.err("Missing input file argument! Please re-run with input file arg. ex: `zig build run -- ./input`", .{});
        return;
    }

    std.debug.print("Starting aoc 11: Plutonian Pebbles. input: '{s}'\n", .{args[1]});

    var stones = std.AutoHashMap(u64, u64).init(allocator);
    defer stones.deinit();
    try read_input(allocator, args[1], &stones);
    const blink_p1 = try run(allocator, &stones, 25);
    std.debug.print("blink p1: {d}\n", .{blink_p1});

    var stones2 = std.AutoHashMap(u64, u64).init(allocator);
    defer stones2.deinit();
    try read_input(allocator, args[1], &stones2);
    const blink_p2 = try run(allocator, &stones2, 75);
    std.debug.print("blink p2: {d}\n", .{blink_p2});
}

test "count_digits tests" {
    try std.testing.expectEqual(4, count_digits(1020));
    try std.testing.expectEqual(1, count_digits(1));
    try std.testing.expectEqual(1, count_digits(0));
    try std.testing.expectEqual(2, count_digits(10));
}

test "split_num tests" {
    const result: [2]u64 = split_num(1020);
    try std.testing.expectEqual(10, result[0]);
    try std.testing.expectEqual(20, result[1]);
    const result2: [2]u64 = split_num(222111);
    try std.testing.expectEqual(222, result2[0]);
    try std.testing.expectEqual(111, result2[1]);
}
