//! AOC day 8 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    map: *std.ArrayList(std.ArrayList(u8)),
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
        while (i < line.len) {
            try row.append(line[i]);
            i += 1;
        }
        try map.append(row);
    }
}

pub fn print_map(
    map: *std.ArrayList(std.ArrayList(u8)),
) void {
    for (map.items) |row| {
        for (row.items) |num| {
            std.debug.print("{c}", .{num});
        }
        std.debug.print("\n", .{});
    }
    std.debug.print("\n", .{});
}

pub fn find_frequency_resonance_count(
    map: *std.ArrayList(std.ArrayList(u8)),
    resonance_positions: *std.AutoHashMap([2]u32, u32),
    frequency: u8,
    freq_r: u32,
    freq_c: u32,
) !u32 {
    var resonants_on_map: u32 = 0;
    for (map.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (r == freq_r and c == freq_c) {
                continue; // can't resonate with self, go to next tile
            }
            if (char == frequency) {
                const row_diff: i32 = @as(i32, @intCast(r)) - @as(i32, @intCast(freq_r));
                const col_diff: i32 = @as(i32, @intCast(c)) - @as(i32, @intCast(freq_c));

                const resonant_r: i32 = @intCast(@as(i32, @intCast(r)) + row_diff);
                const resonant_c: i32 = @intCast(@as(i32, @intCast(c)) + col_diff);

                if (resonant_r >= 0 and
                    resonant_r < map.items.len and
                    resonant_c >= 0 and
                    resonant_c < map.items[r].items.len)
                {
                    const resonant_position = [2]u32{ @as(u32, @intCast(resonant_r)), @as(u32, @intCast(resonant_c)) };
                    if (resonance_positions.contains(resonant_position) == false) {
                        try resonance_positions.put(resonant_position, 1);
                        resonants_on_map += 1;
                    }
                }
            }
        }
    }
    return resonants_on_map;
}

pub fn find_all_resonances_on_map(
    allocator: std.mem.Allocator,
    map: *std.ArrayList(std.ArrayList(u8)),
) !u32 {
    var resonance_positions = std.AutoHashMap([2]u32, u32).init(allocator);
    defer resonance_positions.deinit();
    var all_resonances_on_map_count: u32 = 0;
    for (map.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (char != '.' and char != '#') {
                all_resonances_on_map_count +=
                    try find_frequency_resonance_count(
                    map,
                    &resonance_positions,
                    char,
                    @intCast(r),
                    @intCast(c),
                );
            }
        }
    }

    return all_resonances_on_map_count;
}

// part 2

pub fn find_updated_frequency_resonance_count(
    map: *std.ArrayList(std.ArrayList(u8)),
    resonance_positions: *std.AutoHashMap([2]u32, u32),
    frequency: u8,
    freq_r: u32,
    freq_c: u32,
) !u32 {
    var resonants_on_map: u32 = 0;
    for (map.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (r == freq_r and c == freq_c) {
                continue; // can't resonate with self, go to next tile
            }
            if (char == frequency) {
                if (resonance_positions.contains([2]u32{ @as(u32, @intCast(r)), @as(u32, @intCast(c)) }) == false) {
                    try resonance_positions.put([2]u32{ @as(u32, @intCast(r)), @as(u32, @intCast(c)) }, 1);
                    resonants_on_map += 1;
                }

                const row_diff: i32 = @as(i32, @intCast(r)) - @as(i32, @intCast(freq_r));
                const col_diff: i32 = @as(i32, @intCast(c)) - @as(i32, @intCast(freq_c));

                var resonant_r: i32 = @intCast(@as(i32, @intCast(r)) + row_diff);
                var resonant_c: i32 = @intCast(@as(i32, @intCast(c)) + col_diff);

                while (resonant_r >= 0 and
                    resonant_r < map.items.len and
                    resonant_c >= 0 and
                    resonant_c < map.items[r].items.len)
                {
                    const resonant_position = [2]u32{ @as(u32, @intCast(resonant_r)), @as(u32, @intCast(resonant_c)) };
                    if (resonance_positions.contains(resonant_position) == false) {
                        //std.debug.print("pos: {d},{d}\n", .{ resonant_position[0], resonant_position[1] });
                        try resonance_positions.put(resonant_position, 1);
                        resonants_on_map += 1;
                    }

                    resonant_r = resonant_r + row_diff;
                    resonant_c = resonant_c + col_diff;
                }
            }
        }
    }
    return resonants_on_map;
}

pub fn find_all_updated_resonances_on_map(
    allocator: std.mem.Allocator,
    map: *std.ArrayList(std.ArrayList(u8)),
) !u32 {
    var resonance_positions = std.AutoHashMap([2]u32, u32).init(allocator);
    defer resonance_positions.deinit();
    var all_resonances_on_map_count: u32 = 0;
    for (map.items, 0..) |row, r| {
        for (row.items, 0..) |char, c| {
            if (char != '.' and char != '#') {
                all_resonances_on_map_count +=
                    try find_updated_frequency_resonance_count(
                    map,
                    &resonance_positions,
                    char,
                    @intCast(r),
                    @intCast(c),
                );
            }
        }
    }

    // DEBUG: write resonance points as '#' back to map.
    var res_pos_iterator = resonance_positions.keyIterator();
    while (res_pos_iterator.next()) |pos| {
        if (map.items[pos[0]].items[pos[1]] == '.') {
            map.items[pos[0]].items[pos[1]] = '#';
        }
    }
    print_map(map);

    return all_resonances_on_map_count;
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

    std.debug.print("Starting aoc 8: Resonant Collinearity. input: '{s}'\n", .{args[1]});

    var map = std.ArrayList(std.ArrayList(u8)).init(allocator);
    defer map.deinit();

    try read_input(allocator, args[1], &map);

    print_map(&map);

    const all_resonances_on_map = try find_all_resonances_on_map(allocator, &map);
    std.debug.print("all resonances on map: {d}\n", .{all_resonances_on_map});

    const all_updated_resonances_on_map = try find_all_updated_resonances_on_map(allocator, &map);
    std.debug.print("all updated resonances on map: {d}\n", .{all_updated_resonances_on_map});

    for (map.items) |row| {
        row.deinit();
    }
}
