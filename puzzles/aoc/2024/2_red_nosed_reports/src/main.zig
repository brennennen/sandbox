//! AOC day 2 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

/// Reads a file of rows of numbers into a 2d array list of numbers. Each line
/// represents 1 report to be analyzed.
pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    data: *std.ArrayList(std.ArrayList(i32)),
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
        defer allocator.free(line);
        if (line.len == 0) {
            break; // skip empty lines
        }
        var row = std.ArrayList(i32).init(allocator);
        var i: u32 = 0;
        var left_index: u32 = 0;
        var right_index: u32 = 0;
        while (i < line.len - 1) {
            if (line[i] == ' ') {
                const num: i32 = try std.fmt.parseInt(i32, line[left_index..right_index], 10);
                try row.append(num);
                left_index = i + 1;
            }
            right_index += 1;
            i += 1;
        }
        const num: i32 = try std.fmt.parseInt(i32, line[left_index..right_index], 10);
        try row.append(num);
        try data.append(row);
    }
}

pub fn determine_report_safety(report: *std.ArrayList(i32)) !bool {
    var last_num: i32 = report.items[0];
    const is_increasing: bool = (report.items[1] > last_num);
    for (report.items[1..]) |num| {
        // Rule 1 - All levels are either all increasing or all decreasing
        if (is_increasing) {
            if (num < last_num) {
                return false;
            }
        } else {
            if (num > last_num) {
                return false;
            }
        }
        // Rule 2 - Any two adjacent levels differ by at least 1 and at most 3
        const difference: u32 = @abs(num - last_num);
        if (1 > difference or difference > 3) {
            return false;
        }

        last_num = num;
    }
    return true;
}

// part 1: count the number of safe reports.
pub fn determine_all_reports_safety(data: *std.ArrayList(std.ArrayList(i32))) !u32 {
    var total_safe_reports: u32 = 0;
    for (data.items) |*row| {
        if (try determine_report_safety(row)) {
            total_safe_reports += 1;
        }
    }
    return total_safe_reports;
}

/// part 2: count the number of safe reports, but allow 1 entry to be "bad" and
/// for the report to still be considered safe.
pub fn determine_all_reports_safety_with_dampener(data: *std.ArrayList(std.ArrayList(i32))) !u32 {
    var total_safe_reports: u32 = 0;
    for (data.items) |*row| {
        if (try determine_report_safety(row)) {
            total_safe_reports += 1;
        } else {
            for (0..row.items.len) |i| {
                var new_list = try row.clone();
                defer new_list.deinit();
                _ = new_list.orderedRemove(i);
                if (try determine_report_safety(&new_list)) {
                    total_safe_reports += 1;
                    break;
                }
            }
        }
    }
    return total_safe_reports;
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

    std.debug.print("Starting aoc 2: Red-Nosed Reports. input: '{s}'\n", .{args[1]});

    var data = std.ArrayList(std.ArrayList(i32)).init(allocator);
    defer data.deinit();
    try read_input(allocator, args[1], &data);

    const total_safe_reports = try determine_all_reports_safety(&data);
    std.debug.print("safe reports: {d}\n", .{total_safe_reports});

    const total_safe_with_dampener_reports = try determine_all_reports_safety_with_dampener(&data);
    std.debug.print("safe with dampener reports: {d}\n", .{total_safe_with_dampener_reports});

    for (data.items) |row| { // couldn't figure out how to defer free these "array list in an array list" allocs.
        row.deinit();
    }
}
