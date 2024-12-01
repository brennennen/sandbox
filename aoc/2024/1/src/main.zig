//! AOC day 1 solutions.
//! Run with the input file as the first parameter (relative path to where you run from). Ex:
//! `zig build run ./input`
const std = @import("std");

/// Reads a file of 2 fixed width numbers per line, separated by 3 spaces,
/// into 2 separate array lists.
pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    left_list: *std.ArrayList(u32),
    right_list: *std.ArrayList(u32),
) !void {
    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return;
    };
    defer file.close();

    const delimiter = '\n';
    const max_size = std.math.maxInt(usize);
    while (file.reader().readUntilDelimiterOrEofAlloc(allocator, delimiter, max_size) catch |err| {
        std.log.err("Failed to read line: {s}", .{@errorName(err)});
        return;
    }) |line| {
        if (line.len == 0) {
            break; // skip empty lines
        }
        defer allocator.free(line);
        const left_num: u32 = try std.fmt.parseInt(u32, line[0..5], 10);
        try left_list.append(left_num);
        const right_num: u32 = try std.fmt.parseInt(u32, line[8..13], 10);
        try right_list.append(right_num);
    }
}

// Calculates the total differences between each sorted input list and prints it to stdout.
pub fn find_total_differences(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    stdout: anytype,
) !void {
    std.debug.print("Part 1. Find list total differences. input: {s}\n", .{input_path});

    var left_list = std.ArrayList(u32).init(allocator);
    var right_list = std.ArrayList(u32).init(allocator);
    defer left_list.deinit();
    defer right_list.deinit();

    try read_input(
        allocator,
        input_path,
        &left_list,
        &right_list,
    );

    std.mem.sort(u32, left_list.items, {}, std.sort.asc(u32));
    std.mem.sort(u32, right_list.items, {}, std.sort.asc(u32));

    var total_differences: i64 = 0;

    // "multi-object for loops", kinda niche feature - https://github.com/ziglang/zig/pull/14671
    for (left_list.items, right_list.items) |left, right| {
        var diff: u32 = 0;
        if (left > right) {
            diff = left - right;
        } else {
            diff = right - left;
        }
        total_differences += diff;
    }

    try stdout.print("{d}\n", .{total_differences});
}

/// Finds the accumulated similarity score for each number between the left and right sides as
/// explained in aoc (roughly: score = left_num * occurance in right list).
pub fn find_similarity(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    stdout: anytype,
) !void {
    std.debug.print("Part 2. Find total similarity. input: {s}\n", .{input_path});

    var left_list = std.ArrayList(u32).init(allocator);
    var right_list = std.ArrayList(u32).init(allocator);
    defer left_list.deinit();
    defer right_list.deinit();

    try read_input(
        allocator,
        input_path,
        &left_list,
        &right_list,
    );

    std.mem.sort(u32, left_list.items, {}, std.sort.asc(u32));
    std.mem.sort(u32, right_list.items, {}, std.sort.asc(u32));

    var total_similarity: u32 = 0;
    for (left_list.items) |left| {
        var occurances: u32 = 0;
        var last_right_index: usize = 0;
        for (right_list.items[last_right_index..]) |right| {
            last_right_index += 1; // because the list is sorted, we don't have to start at 0 on the right list each time, we can start at our last index to save going over values less than the left num.
            if (left == right) {
                occurances += 1;
            } else if (left < right) {
                break; // because the list is sorted, we can exit the right list early if we hit a number greater than left.
            }
        }
        total_similarity += left * occurances;
    }

    try stdout.print("{d}\n", .{total_similarity});
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

    const stdout_file = std.io.getStdOut().writer();
    var buffered_writer = std.io.bufferedWriter(stdout_file);
    const stdout = buffered_writer.writer();

    std.debug.print("Starting aoc 1: historian hysteria. input: {s}\n", .{args[1]});

    try find_total_differences(allocator, args[1], stdout);
    try buffered_writer.flush();
    try find_similarity(allocator, args[1], stdout);
    try buffered_writer.flush();
}
