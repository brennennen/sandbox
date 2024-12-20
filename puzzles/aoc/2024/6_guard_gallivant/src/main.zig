//! AOC day 6 solutions.
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

pub fn print_map(map: std.ArrayList(std.ArrayList(u8))) !void {
    for (map.items) |row| {
        for (row.items) |num| {
            std.debug.print("{c}", .{num});
        }
        std.debug.print("\n", .{});
    }
    std.debug.print("\n", .{});
}

pub fn find_guard_position(map: std.ArrayList(std.ArrayList(u8))) ![2]u32 {
    for (map.items, 0..) |row, r| {
        for (row.items, 0..) |num, c| {
            if (num == '^' or num == 'v' or num == '>' or num == '<') {
                return [2]u32{ @intCast(r), @intCast(c) };
            }
        }
    }
    return [2]u32{ 0, 0 };
}

/// Performs a single step of guard movement.
/// Returns true if the guard is still within the map.
pub fn step_guard_and_mark(map: *std.ArrayList(std.ArrayList(u8)), guard_position: *[2]u32) !bool {
    const r = guard_position[0];
    const c = guard_position[1];
    const guard_char = map.items[r].items[c];

    if (guard_char == '^') {
        if (r > 0) {
            if (map.items[r - 1].items[c] == '#') {
                map.items[r].items[c] = '>';
            } else {
                map.items[r].items[c] = 'X';
                map.items[r - 1].items[c] = '^';
                guard_position[0] = r - 1;
            }
        } else {
            map.items[r].items[c] = 'X';
            return false;
        }
    } else if (guard_char == '>') {
        if (c + 1 < map.items[r].items.len) {
            if (map.items[r].items[c + 1] == '#') {
                map.items[r].items[c] = 'v';
            } else {
                map.items[r].items[c] = 'X';
                map.items[r].items[c + 1] = '>';
                guard_position[1] = c + 1;
            }
        } else {
            map.items[r].items[c] = 'X';
            return false;
        }
    } else if (guard_char == 'v') {
        if (r + 1 < map.items.len) {
            if (map.items[r + 1].items[c] == '#') {
                map.items[r].items[c] = '<';
            } else {
                map.items[r].items[c] = 'X';
                map.items[r + 1].items[c] = 'v';
                guard_position[0] = r + 1;
            }
        } else {
            map.items[r].items[c] = 'X';
            return false;
        }
    } else if (guard_char == '<') {
        if (c > 0) {
            if (map.items[r].items[c - 1] == '#') {
                map.items[r].items[c] = '^';
            } else {
                map.items[r].items[c] = 'X';
                map.items[r].items[c - 1] = '<';
                guard_position[1] = c - 1;
            }
        } else {
            map.items[r].items[c] = 'X';
            return false;
        }
    } else {
        std.debug.print("Failed to traverse maze. lost guard position!\n", .{});
        return false;
    }
    return true;
}

pub fn count_char(map: std.ArrayList(std.ArrayList(u8)), char: u8) !u32 {
    var char_count: u32 = 0;
    for (map.items) |row| {
        for (row.items) |num| {
            if (num == char) {
                char_count += 1;
            }
        }
    }
    return char_count;
}

pub fn simulate_simple_guard_movements(map: *std.ArrayList(std.ArrayList(u8))) !u32 {
    try print_map(map.*);
    var guard_movements: u32 = 0;
    var guard_position = [2]u32{ 0, 0 };
    guard_position = try find_guard_position(map.*);
    var sentinel: u32 = 0;
    while (try step_guard_and_mark(map, &guard_position)) {
        if (sentinel > 8192) {
            std.debug.print("sentinel check failed!\n", .{});
            break;
        }
        sentinel += 1;
    }
    guard_movements = try count_char(map.*, 'X');
    try print_map(map.*);
    return guard_movements;
}

// Part 2 chaos
const GuardState = struct {
    r: u32,
    c: u32,
    char: u8,
};

pub const GuardStateContext = struct {
    pub fn hash(self: @This(), state: GuardState) u64 {
        _ = self;
        return (state.r * 1_000_000) + (state.c * 1000) + state.char;
    }
    pub fn eql(self: @This(), left: GuardState, right: GuardState) bool {
        _ = self;
        return (left.r == right.r and
            left.c == right.c and
            left.char == right.char);
    }
};

/// Performs a single step of guard movement.
/// Returns true if the guard is still within the map.
pub fn step_guard(
    map: *std.ArrayList(std.ArrayList(u8)),
    guard_position: *[2]u32,
) !bool {
    const r = guard_position[0];
    const c = guard_position[1];
    const guard_char = map.items[r].items[c];

    if (guard_char == '^') {
        if (r > 0) {
            if (map.items[r - 1].items[c] == '#') {
                map.items[r].items[c] = '>';
            } else {
                map.items[r].items[c] = '.';
                map.items[r - 1].items[c] = '^';
                guard_position[0] = r - 1;
            }
        } else {
            map.items[r].items[c] = '.';
            return false;
        }
    } else if (guard_char == '>') {
        if (c + 1 < map.items[r].items.len) {
            if (map.items[r].items[c + 1] == '#') {
                map.items[r].items[c] = 'v';
            } else {
                map.items[r].items[c] = '.';
                map.items[r].items[c + 1] = '>';
                guard_position[1] = c + 1;
            }
        } else {
            map.items[r].items[c] = '.';
            return false;
        }
    } else if (guard_char == 'v') {
        if (r + 1 < map.items.len) {
            if (map.items[r + 1].items[c] == '#') {
                map.items[r].items[c] = '<';
            } else {
                map.items[r].items[c] = '.';
                map.items[r + 1].items[c] = 'v';
                guard_position[0] = r + 1;
            }
        } else {
            map.items[r].items[c] = '.';
            return false;
        }
    } else if (guard_char == '<') {
        if (c > 0) {
            if (map.items[r].items[c - 1] == '#') {
                map.items[r].items[c] = '^';
            } else {
                map.items[r].items[c] = '.';
                map.items[r].items[c - 1] = '<';
                guard_position[1] = c - 1;
            }
        } else {
            map.items[r].items[c] = '.';
            return false;
        }
    } else {
        std.debug.print(
            "Failed to traverse maze. lost guard position! ({d}, {d})\n",
            .{ r, c },
        );
        try print_map(map.*);
        return false;
    }
    return true;
}

pub fn build_normal_guard_states(
    map: *std.ArrayList(std.ArrayList(u8)),
    states_hashmap: *std.HashMap(GuardState, u8, GuardStateContext, std.hash_map.default_max_load_percentage),
) !void {
    var guard_position = [2]u32{ 0, 0 };
    guard_position = try find_guard_position(map.*);
    while (try step_guard(map, &guard_position)) {
        const guard_state = GuardState{
            .r = guard_position[0],
            .c = guard_position[1],
            .char = map.items[guard_position[0]].items[guard_position[1]],
        };
        try states_hashmap.put(guard_state, 1);
    }
}

pub fn find_guard_loop_options(
    allocator: std.mem.Allocator,
    map: *std.ArrayList(std.ArrayList(u8)),
) !u32 {
    const initial_guard_position = try find_guard_position(map.*);
    var potential_obstacles = std.HashMap(GuardState, u8, GuardStateContext, std.hash_map.default_max_load_percentage).init(allocator);
    defer potential_obstacles.deinit();

    // Zig doesn't have a "hash set" type yet in the standard library, using hashmap with "1" for value.
    var normal_states = std.HashMap(GuardState, u8, GuardStateContext, std.hash_map.default_max_load_percentage).init(allocator);
    defer normal_states.deinit();
    try build_normal_guard_states(map, &normal_states);

    var hash_iterator = normal_states.keyIterator();
    while (hash_iterator.next()) |state| {
        var deviation_states = std.HashMap(GuardState, u8, GuardStateContext, std.hash_map.default_max_load_percentage).init(allocator);
        defer deviation_states.deinit();

        var obstacle_pos: [2]u32 = .{ 0, 0 };
        if (state.char == '^') {
            if (state.r > 0 and
                map.items[state.r - 1].items[state.c] != '#')
            {
                map.items[state.r].items[state.c] = '>';
                obstacle_pos = .{ state.r - 1, state.c };
            } else {
                continue;
            }
        } else if (state.char == '>') {
            if (state.c + 1 < map.items[state.r].items.len and
                map.items[state.r].items[state.c + 1] != '#')
            {
                map.items[state.r].items[state.c] = 'v';
                obstacle_pos = .{ state.r, state.c + 1 };
            } else {
                continue;
            }
        } else if (state.char == 'v') {
            if (state.r + 1 < map.items.len and
                map.items[state.r + 1].items[state.c] != '#')
            {
                map.items[state.r].items[state.c] = '<';
                obstacle_pos = .{ state.r + 1, state.c };
            } else {
                continue;
            }
        } else if (state.char == '<') {
            if (state.c > 0 and
                map.items[state.r].items[state.c - 1] != '#')
            {
                map.items[state.r].items[state.c] = '^';
                obstacle_pos = .{ state.r, state.c - 1 };
            } else {
                continue;
            }
        }

        map.items[initial_guard_position[0]].items[initial_guard_position[1]] = '^';
        map.items[obstacle_pos[0]].items[obstacle_pos[1]] = '#'; // insert the potential blocker

        var guard_position: [2]u32 = .{ initial_guard_position[0], initial_guard_position[1] };
        while (try step_guard(map, &guard_position)) {
            const temp_state = GuardState{
                .r = guard_position[0],
                .c = guard_position[1],
                .char = map.items[guard_position[0]].items[guard_position[1]],
            };

            if (deviation_states.contains(temp_state) == true) {
                try potential_obstacles.put(GuardState{
                    .r = obstacle_pos[0],
                    .c = obstacle_pos[1],
                    .char = 'O',
                }, 1);
                break;
            }
            try deviation_states.put(temp_state, 1);
        }

        map.items[obstacle_pos[0]].items[obstacle_pos[1]] = '.'; // remove the potential blocker
    }

    var obstacles_iterator = potential_obstacles.keyIterator();
    while (obstacles_iterator.next()) |state| {
        map.items[state.r].items[state.c] = 'O';
    }
    try print_map(map.*);

    return potential_obstacles.count();
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

    std.debug.print("Starting aoc 6: guard gallivant. input: '{s}'\n", .{args[1]});

    var map = std.ArrayList(std.ArrayList(u8)).init(allocator);
    defer map.deinit();
    var map2 = std.ArrayList(std.ArrayList(u8)).init(allocator);
    defer map2.deinit();

    try read_input(allocator, args[1], &map);
    try read_input(allocator, args[1], &map2);

    const guard_movements = try simulate_simple_guard_movements(&map);
    std.debug.print("Part 1 - guard movements: {d}\n", .{guard_movements});

    const loop_options = try find_guard_loop_options(allocator, &map2);
    std.debug.print("Part 2 - loop options: {d}\n", .{loop_options});

    for (map.items, map2.items) |row, row2| {
        row.deinit();
        row2.deinit();
    }
}
