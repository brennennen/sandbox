//! AOC day 5 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

/// Input had 2 sections that require different parsing methods separated by a blank new line.
/// Part1 is 2 pipe separated 2 digit numbers per line. ex: "20|24"
/// Part2 is a csv of 2 digit numbers per line.
pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    page_ordering_rules: *std.ArrayList([2]u8),
    updates: *std.ArrayList(std.ArrayList(u8)),
) !void {
    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return;
    };
    defer file.close();

    const delimiter = '\n';
    const max_size = std.math.maxInt(usize);

    var part1_mode: bool = true;

    while (file.reader().readUntilDelimiterOrEofAlloc(allocator, delimiter, max_size) catch |err| {
        std.log.err("Failed to read line: {s}", .{@errorName(err)});
        return;
    }) |line| {
        defer allocator.free(line);

        if (part1_mode and line.len == 0) {
            part1_mode = false;
            //std.debug.print("found empty line, starting part 2\n", .{});
            continue;
        }

        if (part1_mode) {
            // Part1 is 2 pipe separated 2 digit numbers per line. ex: "20|24"
            var rule: [2]u8 = .{ 0, 0 };
            const left: u8 = try std.fmt.parseInt(u8, line[0..2], 10);
            const right: u8 = try std.fmt.parseInt(u8, line[3..5], 10);
            rule[0] = left;
            rule[1] = right;
            try page_ordering_rules.append(rule);
        } else {
            // Part2 is a csv of 2 digit numbers per line.
            var update = std.ArrayList(u8).init(allocator);
            var j: u32 = 0;
            var left_index: u32 = 0;
            var right_index: u32 = 0;
            while (j < line.len - 1) {
                if (line[j] == ',') {
                    const num: u8 = try std.fmt.parseInt(u8, line[left_index..right_index], 10);
                    try update.append(num);
                    left_index = j + 1;
                }
                right_index += 1;
                j += 1;
            }
            right_index += 1;
            const num: u8 = try std.fmt.parseInt(u8, line[left_index..right_index], 10);
            try update.append(num);
            try updates.append(update);
        }
    }
}

pub fn is_update_order_correct(
    page_ordering_rules: *std.ArrayList([2]u8),
    update: std.ArrayList(u8),
) !bool {
    for (update.items, 0..) |page, page_index| {
        for (page_ordering_rules.items) |rule| {
            // If the page matches first number in rule "x|y", make sure it comes before the other page.
            if (page == rule[0]) {
                for (update.items, 0..) |page2, page2_index| {
                    if (page2 == rule[1]) {
                        if (page_index >= page2_index) {
                            return false;
                        }
                    }
                }
            }
            // If the page matches second number in rule "x|y", make sure it comes after the other page.
            if (page == rule[1]) {
                for (update.items, 0..) |page2, page2_index| {
                    if (page2 == rule[0]) {
                        if (page_index <= page2_index) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

pub fn get_correct_update_middles(
    page_ordering_rules: *std.ArrayList([2]u8),
    updates: *std.ArrayList(std.ArrayList(u8)),
) !u32 {
    var correct_update_accum_middles: u32 = 0;
    for (updates.items) |update| {
        if (try is_update_order_correct(page_ordering_rules, update)) {
            const middle_page_index: usize = (update.items.len / 2);
            const middle_page = update.items[middle_page_index];
            correct_update_accum_middles += middle_page;
        }
    }
    return correct_update_accum_middles;
}

pub fn correct_update(
    page_ordering_rules: *std.ArrayList([2]u8),
    update: *std.ArrayList(u8),
) !void {
    while (try is_update_order_correct(page_ordering_rules, update.*) == false) {
        for (update.items, 0..) |page, page_index| {
            for (page_ordering_rules.items) |rule| {
                // If the page matches first number in rule "x|y", make sure it comes before the other page.
                if (page == rule[0]) {
                    for (update.items, 0..) |page2, page2_index| {
                        if (page2 == rule[1]) {
                            if (page_index >= page2_index) {
                                const temp = update.items[page_index];
                                update.items[page_index] = update.items[page2_index];
                                update.items[page2_index] = temp;
                            }
                        }
                    }
                }
                // If the page matches second number in rule "x|y", make sure it comes after the other page.
                if (page == rule[1]) {
                    for (update.items, 0..) |page2, page2_index| {
                        if (page2 == rule[0]) {
                            if (page_index <= page2_index) {
                                const temp = update.items[page_index];
                                update.items[page_index] = update.items[page2_index];
                                update.items[page2_index] = temp;
                            }
                        }
                    }
                }
            }
        }
    }
}

pub fn get_fixed_incorrect_update_middles(
    page_ordering_rules: *std.ArrayList([2]u8),
    updates: *std.ArrayList(std.ArrayList(u8)),
) !u32 {
    var fixed_incorrect_update_accum_middles: u32 = 0;
    for (updates.items) |*update| {
        if (try is_update_order_correct(page_ordering_rules, update.*) == false) {
            try correct_update(page_ordering_rules, update);

            const middle_page_index: usize = (update.items.len / 2);
            const middle_page = update.items[middle_page_index];
            fixed_incorrect_update_accum_middles += middle_page;
        }
    }
    return fixed_incorrect_update_accum_middles;
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

    std.debug.print("Starting aoc 4: ceres search. input: '{s}'\n", .{args[1]});

    var page_ordering_rules = std.ArrayList([2]u8).init(allocator);
    defer page_ordering_rules.deinit();
    var updates = std.ArrayList(std.ArrayList(u8)).init(allocator);
    defer updates.deinit();
    try read_input(
        allocator,
        args[1],
        &page_ordering_rules,
        &updates,
    );

    const correct_middles: u32 = try get_correct_update_middles(
        &page_ordering_rules,
        &updates,
    );
    std.debug.print("correct_middles: {d}\n", .{correct_middles});

    const fixed_incorrect_middles: u32 = try get_fixed_incorrect_update_middles(
        &page_ordering_rules,
        &updates,
    );
    std.debug.print("fixed_incorrect_middles: {d}\n", .{fixed_incorrect_middles});

    for (updates.items) |row| {
        row.deinit();
    }
}
