//! AOC day 10 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

const Coord = struct {
    r: u8,
    c: u8,
};

const Map = struct {
    cells: std.ArrayList(std.ArrayList(u8)),
    width: u8,
    height: u8,

    pub fn init(allocator: std.mem.Allocator) !Map {
        return .{
            .cells = std.ArrayList(std.ArrayList(u8)).init(allocator),
            .width = 0,
            .height = 0,
        };
    }

    pub fn read_file(
        self: *Map,
        allocator: std.mem.Allocator,
        input_path: [:0]const u8,
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
                try row.append(line[i] - '0');
                i += 1;
            }
            try self.cells.append(row);
        }
        self.height = @intCast(self.cells.items.len);
        self.width = @intCast(self.cells.items[0].items.len);
    }

    pub fn deinit(self: *Map) void {
        for (self.cells.items) |row| {
            row.deinit();
        }
        self.cells.deinit();
    }

    pub fn debug_print(self: *Map) void {
        for (self.cells.items) |row| {
            for (row.items) |num| {
                std.debug.print("{d}", .{num});
            }
            std.debug.print("\n", .{});
        }
        std.debug.print("\n", .{});
    }
};

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
            try row.append(line[i] - '0');
            i += 1;
        }
        try map.append(row);
    }
}

pub fn find_trail_heads(
    map: std.ArrayList(std.ArrayList(u8)),
    heads: std.ArrayList(Coord),
) void {
    for (map, 0..) |row, r| {
        for (row, 0..) |cell, c| {
            if (cell == 0) {
                heads.append(Coord{ .r = r, .c = c });
            }
        }
    }
}

pub fn find_trail_head_score(
    allocator: std.mem.Allocator,
    map: Map,
    trail_head: Coord,
) !u32 {
    var trail_summits_reached = std.AutoHashMap(Coord, u8).init(allocator);
    defer trail_summits_reached.deinit();
    //var trail_score = 0;

    var stack = std.ArrayList(Coord).init(allocator);
    defer stack.deinit();
    try stack.append(trail_head);

    while (stack.items.len != 0) {
        const cur = stack.pop();
        const cur_val = map.cells.items[cur.r].items[cur.c];

        if (cur_val == 9) {
            try trail_summits_reached.put(cur, 1);
            continue;
        }

        // up
        if (cur.r >= 1) {
            const up = Coord{ .r = cur.r - 1, .c = cur.c };
            const up_val = map.cells.items[up.r].items[up.c];
            if (cur_val + 1 == up_val) {
                try stack.append(up);
            }
        }
        // right
        if (cur.c < map.width - 1) {
            const right = Coord{ .r = cur.r, .c = cur.c + 1 };
            const right_val = map.cells.items[right.r].items[right.c];
            if (cur_val + 1 == right_val) {
                try stack.append(right);
            }
        }

        // down
        if (cur.r < map.height - 1) {
            const down = Coord{ .r = cur.r + 1, .c = cur.c };
            const down_val = map.cells.items[down.r].items[down.c];
            if (cur_val + 1 == down_val) {
                try stack.append(down);
            }
        }
        // left
        if (cur.c >= 1) {
            const left = Coord{ .r = cur.r, .c = cur.c - 1 };
            const left_val = map.cells.items[left.r].items[left.c];
            if (cur_val + 1 == left_val) {
                try stack.append(left);
            }
        }
    }
    return trail_summits_reached.count();
}

pub fn find_total_trails_score(
    allocator: std.mem.Allocator,
    map: Map,
) !u32 {
    var total_score: u32 = 0;
    for (map.cells.items, 0..) |row, r| {
        for (row.items, 0..) |cell, c| {
            if (cell == 0) {
                total_score += try find_trail_head_score(
                    allocator,
                    map,
                    Coord{ .r = @intCast(r), .c = @intCast(c) },
                );
            }
        }
    }
    return total_score;
}

pub fn find_trail_head_distinct_score(
    allocator: std.mem.Allocator,
    map: Map,
    trail_head: Coord,
) !u32 {
    var trail_score: u32 = 0;

    var stack = std.ArrayList(Coord).init(allocator);
    defer stack.deinit();
    try stack.append(trail_head);

    while (stack.items.len != 0) {
        const cur = stack.pop();
        const cur_val = map.cells.items[cur.r].items[cur.c];

        if (cur_val == 9) {
            trail_score += 1;
            continue;
        }

        // up
        if (cur.r >= 1) {
            const up = Coord{ .r = cur.r - 1, .c = cur.c };
            const up_val = map.cells.items[up.r].items[up.c];
            if (cur_val + 1 == up_val) {
                try stack.append(up);
            }
        }
        // right
        if (cur.c < map.width - 1) {
            const right = Coord{ .r = cur.r, .c = cur.c + 1 };
            const right_val = map.cells.items[right.r].items[right.c];
            if (cur_val + 1 == right_val) {
                try stack.append(right);
            }
        }

        // down
        if (cur.r < map.height - 1) {
            const down = Coord{ .r = cur.r + 1, .c = cur.c };
            const down_val = map.cells.items[down.r].items[down.c];
            if (cur_val + 1 == down_val) {
                try stack.append(down);
            }
        }
        // left
        if (cur.c >= 1) {
            const left = Coord{ .r = cur.r, .c = cur.c - 1 };
            const left_val = map.cells.items[left.r].items[left.c];
            if (cur_val + 1 == left_val) {
                try stack.append(left);
            }
        }
    }
    return trail_score;
}

pub fn find_total_trails_distinc_score(
    allocator: std.mem.Allocator,
    map: Map,
) !u32 {
    var total_score: u32 = 0;
    for (map.cells.items, 0..) |row, r| {
        for (row.items, 0..) |cell, c| {
            if (cell == 0) {
                total_score += try find_trail_head_distinct_score(
                    allocator,
                    map,
                    Coord{ .r = @intCast(r), .c = @intCast(c) },
                );
            }
        }
    }
    return total_score;
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

    std.debug.print("Starting aoc 10: Hoof It. input: '{s}'\n", .{args[1]});

    var map = try Map.init(allocator);
    defer map.deinit();
    try map.read_file(allocator, args[1]);
    map.debug_print();

    const total_trail_scores = try find_total_trails_score(allocator, map);
    std.debug.print("p1: {d}\n", .{total_trail_scores});

    const total_trail_distinct_scores = try find_total_trails_distinc_score(allocator, map);
    std.debug.print("p2: {d}\n", .{total_trail_distinct_scores});
}
