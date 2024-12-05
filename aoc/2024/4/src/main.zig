//! AOC day 4 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    data: *std.ArrayList(std.ArrayList(u8)),
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
        var row = std.ArrayList(u8).init(allocator);
        var i: u32 = 0;
        while (i < line.len - 1) {
            try row.append(line[i]);
            i += 1;
        }
        try data.append(row);
    }
}

/// Find the occurances of "XMAS" in the word grid, the word can be horizontal, vertical,
/// diagonal, backwards, or even overlapping other words.
pub fn ceres_xmas_search(data: *std.ArrayList(std.ArrayList(u8))) !u32 {
    var xmas_count: u32 = 0;
    for (data.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (char == 'X') {
                // check right
                if (c + 3 < row.items.len and
                    row.items[c + 1] == 'M' and
                    row.items[c + 2] == 'A' and
                    row.items[c + 3] == 'S')
                {
                    //std.debug.print("found xmas right [{d}, {d}]\n", .{ r, c });
                    xmas_count += 1;
                }

                // check left
                if (c >= 3 and
                    row.items[c - 1] == 'M' and
                    row.items[c - 2] == 'A' and
                    row.items[c - 3] == 'S')
                {
                    //std.debug.print("found xmas left [{d}, {d}]\n", .{ r, c });
                    xmas_count += 1;
                }

                // check up
                if (r >= 3 and
                    data.items[r - 1].items[c] == 'M' and
                    data.items[r - 2].items[c] == 'A' and
                    data.items[r - 3].items[c] == 'S')
                {
                    //std.debug.print("found xmas up [{d}, {d}]\n", .{ r, c });
                    xmas_count += 1;
                }

                // check down
                if (r + 3 < data.items.len and
                    data.items[r + 1].items[c] == 'M' and
                    data.items[r + 2].items[c] == 'A' and
                    data.items[r + 3].items[c] == 'S')
                {
                    //std.debug.print("found xmas down [{d}, {d}]\n", .{ r, c });
                    xmas_count += 1;
                }

                // diagonal up right
                if (r >= 3 and c + 3 < row.items.len) {
                    if (data.items[r - 1].items[c + 1] == 'M' and
                        data.items[r - 2].items[c + 2] == 'A' and
                        data.items[r - 3].items[c + 3] == 'S')
                    {
                        //std.debug.print("found xmas diagonal up right [{d}, {d}]\n", .{ r, c });
                        xmas_count += 1;
                    }
                }

                // diagonal down right
                if (r + 3 < data.items.len and c + 3 < row.items.len) {
                    if (data.items[r + 1].items[c + 1] == 'M' and
                        data.items[r + 2].items[c + 2] == 'A' and
                        data.items[r + 3].items[c + 3] == 'S')
                    {
                        //std.debug.print("found xmas diagonal down right [{d}, {d}]\n", .{ r, c });
                        xmas_count += 1;
                    }
                }

                // diagonal up left
                if (r >= 3 and c >= 3) {
                    if (data.items[r - 1].items[c - 1] == 'M' and
                        data.items[r - 2].items[c - 2] == 'A' and
                        data.items[r - 3].items[c - 3] == 'S')
                    {
                        //std.debug.print("found xmas diagonal down right [{d}, {d}]\n", .{ r, c });
                        xmas_count += 1;
                    }
                }

                // diagonal down left
                if (r + 3 < data.items.len and c >= 3) {
                    if (data.items[r + 1].items[c - 1] == 'M' and
                        data.items[r + 2].items[c - 2] == 'A' and
                        data.items[r + 3].items[c - 3] == 'S')
                    {
                        //std.debug.print("found xmas diagonal down right [{d}, {d}]\n", .{ r, c });
                        xmas_count += 1;
                    }
                }
            }
        }
    }
    return xmas_count;
}

/// Find the occurances of "MAS" in the shape of an "X".
/// 4 Permutations:
///  M.S   S.M   M.M   S.S
///  .A.   .A.   .A.   .A.
///  M.S   S.M   S.S   M.M
pub fn ceres_x_mas_search(data: *std.ArrayList(std.ArrayList(u8))) !u32 {
    var x_mas_count: u32 = 0;
    for (data.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (char == 'A') {
                if (r >= 1 and
                    c >= 1 and
                    r + 1 < data.items.len and
                    c + 1 < row.items.len)
                {
                    if (data.items[r + 1].items[c - 1] == 'M' and
                        data.items[r - 1].items[c - 1] == 'M' and
                        data.items[r + 1].items[c + 1] == 'S' and
                        data.items[r - 1].items[c + 1] == 'S')
                    {
                        x_mas_count += 1;
                    }

                    if (data.items[r + 1].items[c - 1] == 'S' and
                        data.items[r - 1].items[c - 1] == 'S' and
                        data.items[r + 1].items[c + 1] == 'M' and
                        data.items[r - 1].items[c + 1] == 'M')
                    {
                        x_mas_count += 1;
                    }

                    if (data.items[r + 1].items[c - 1] == 'S' and
                        data.items[r - 1].items[c - 1] == 'M' and
                        data.items[r + 1].items[c + 1] == 'S' and
                        data.items[r - 1].items[c + 1] == 'M')
                    {
                        x_mas_count += 1;
                    }

                    if (data.items[r + 1].items[c - 1] == 'M' and
                        data.items[r - 1].items[c - 1] == 'S' and
                        data.items[r + 1].items[c + 1] == 'M' and
                        data.items[r - 1].items[c + 1] == 'S')
                    {
                        x_mas_count += 1;
                    }
                }
            }
        }
    }
    return x_mas_count;
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

    var data = std.ArrayList(std.ArrayList(u8)).init(allocator);
    defer data.deinit();
    try read_input(allocator, args[1], &data);

    const xmas_search_result = try ceres_xmas_search(&data);
    std.debug.print("Part 1 - xmas_search: {d}\n", .{xmas_search_result});

    const x_mas_search_result = try ceres_x_mas_search(&data);
    std.debug.print("Part 2 - x_mas_search: {d}\n", .{x_mas_search_result});

    for (data.items) |row| {
        row.deinit();
    }
}
