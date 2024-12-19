//! AOC day 12 solutions.
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
                try row.append(line[i]);
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

    pub fn contains_coord(self: *const Map, coord: Coord) bool {
        if (coord.r >= 0 and
            coord.c >= 0 and
            coord.r < self.height and
            coord.c < self.width)
        {
            return true;
        }
        return false;
    }

    pub fn get(self: *const Map, coord: Coord) ?u8 {
        if (self.contains_coord(coord)) {
            return self.cells.items[coord.r].items[coord.c];
        } else {
            return null;
        }
    }

    pub fn debug_print(self: *Map) void {
        for (self.cells.items) |row| {
            for (row.items) |num| {
                std.debug.print("{c}", .{num});
            }
            std.debug.print("\n", .{});
        }
        std.debug.print("\n", .{});
    }
};

pub fn find_contiguous(
    allocator: std.mem.Allocator,
    map: *const Map,
    all_contiguous: *std.ArrayList(std.ArrayList(Coord)),
    visited_cells: *std.AutoHashMap(Coord, u8),
    cell_value: u8,
    cell_coord: Coord,
) !void {
    var contiguous = std.ArrayList(Coord).init(allocator);
    try contiguous.append(cell_coord);

    var next_cell_coords = std.AutoArrayHashMap(Coord, u8).init(allocator);
    defer next_cell_coords.deinit();
    try next_cell_coords.put(cell_coord, 1);

    while (next_cell_coords.count() > 0) {
        const entry = next_cell_coords.pop();
        const coord = entry.key;
        if (visited_cells.contains(coord)) {
            continue;
        }
        // north
        if (coord.r >= 1) {
            const north = Coord{ .r = coord.r - 1, .c = coord.c };
            if (map.contains_coord(north) and
                !visited_cells.contains(north) and
                map.get(north).? == cell_value and
                !next_cell_coords.contains(north))
            {
                try contiguous.append(north);
                try next_cell_coords.put(north, 1);
            }
        }
        // east
        {
            const east = Coord{ .r = coord.r, .c = coord.c + 1 };
            if (map.contains_coord(east) and
                !visited_cells.contains(east) and
                map.get(east).? == cell_value and
                !next_cell_coords.contains(east))
            {
                try contiguous.append(east);
                try next_cell_coords.put(east, 1);
            }
        }
        // south
        {
            const south = Coord{ .r = coord.r + 1, .c = coord.c };
            if (map.contains_coord(south) and
                !visited_cells.contains(south) and
                map.get(south).? == cell_value and
                !next_cell_coords.contains(south))
            {
                try contiguous.append(south);
                try next_cell_coords.put(south, 1);
            }
        }
        // west
        if (coord.c >= 1) {
            const west = Coord{ .r = coord.r, .c = coord.c - 1 };
            if (map.contains_coord(west) and
                !visited_cells.contains(west) and
                map.get(west).? == cell_value and
                !next_cell_coords.contains(west))
            {
                try contiguous.append(west);
                try next_cell_coords.put(west, 1);
            }
        }
        try visited_cells.put(coord, 1);
    }

    if (contiguous.items.len > 0) {
        try all_contiguous.append(contiguous);
    }
}

pub fn find_all_congtiguous_cells(
    allocator: std.mem.Allocator,
    map: *const Map,
    contiguous_cells: *std.ArrayList(std.ArrayList(Coord)),
) !void {
    var visited_cells = std.AutoHashMap(Coord, u8).init(allocator);
    defer visited_cells.deinit();

    for (map.cells.items, 0..) |row, r| {
        for (row.items, 0..) |cell, c| {
            const coord = Coord{ .r = @intCast(r), .c = @intCast(c) };
            if (visited_cells.contains(coord) == false) {
                try find_contiguous(
                    allocator,
                    map,
                    contiguous_cells,
                    &visited_cells,
                    cell,
                    coord,
                );
            }
        }
    }
}

pub fn count_contiguous_perimeter(
    map: *const Map,
    contiguous: *const std.ArrayList(Coord),
) u32 {
    const cell_value = map.get(contiguous.items[0]).?;
    var perimeter: u32 = 0;

    for (contiguous.items) |coord| {
        // north
        if (coord.r <= 0) {
            perimeter += 1;
        } else {
            const north = Coord{ .r = coord.r - 1, .c = coord.c };
            if (map.contains_coord(north) and cell_value != map.get(north).?) {
                perimeter += 1;
            }
        }
        // east
        if (coord.c >= map.width - 1) {
            perimeter += 1;
        } else {
            const east = Coord{ .r = coord.r, .c = coord.c + 1 };
            if (map.contains_coord(east) and cell_value != map.get(east).?) {
                perimeter += 1;
            }
        }
        // south
        if (coord.r >= map.height - 1) {
            perimeter += 1;
        } else {
            const south = Coord{ .r = coord.r + 1, .c = coord.c };
            if (map.contains_coord(south) and cell_value != map.get(south).?) {
                perimeter += 1;
            }
        }
        // west
        if (coord.c <= 0) {
            perimeter += 1;
        } else {
            const west = Coord{ .r = coord.r, .c = coord.c - 1 };
            if (map.contains_coord(west) and cell_value != map.get(west).?) {
                perimeter += 1;
            }
        }
    }
    return perimeter;
}

// For part 1, total fence cost is the perimeter * area
pub fn find_p1_total_fence_cost(
    allocator: std.mem.Allocator,
    map: *const Map,
) !usize {
    var all_contiguous = std.ArrayList(std.ArrayList(Coord)).init(allocator);
    defer all_contiguous.deinit();

    try find_all_congtiguous_cells(
        allocator,
        map,
        &all_contiguous,
    );

    var total_fence_cost: usize = 0;
    for (all_contiguous.items) |contiguous| {
        const perimeter = count_contiguous_perimeter(map, &contiguous);
        total_fence_cost += contiguous.items.len * perimeter;
    }

    for (all_contiguous.items) |cells| {
        cells.deinit();
    }

    return total_fence_cost;
}

/// Finds a count of contiguous cells the cell at the initial coord is a part of.
/// Also marks each cell in the contiguous group as visited.
pub fn find_contiguous_group_count(
    allocator: std.mem.Allocator,
    initial_coord: Coord,
    coords: *const std.AutoHashMap(Coord, u8),
    visited: *std.AutoHashMap(Coord, u8),
) !u32 {
    if (visited.contains(initial_coord)) {
        return 0;
    }

    var next_coords = std.AutoArrayHashMap(Coord, u8).init(allocator);
    defer next_coords.deinit();
    try next_coords.put(initial_coord, 1);

    while (next_coords.count() > 0) {
        const entry = next_coords.pop();
        const coord = entry.key;
        if (visited.contains(coord)) {
            continue;
        }

        // north
        if (coord.r >= 1) {
            const north = Coord{ .r = coord.r - 1, .c = coord.c };
            if (coords.contains(north) and
                !visited.contains(north) and
                !next_coords.contains(north))
            {
                try next_coords.put(north, 1);
            }
        }
        // east
        {
            const east = Coord{ .r = coord.r, .c = coord.c + 1 };
            if (coords.contains(east) and
                !visited.contains(east) and
                !next_coords.contains(east))
            {
                try next_coords.put(east, 1);
            }
        }
        // south
        {
            const south = Coord{ .r = coord.r + 1, .c = coord.c };
            if (coords.contains(south) and
                !visited.contains(south) and
                !next_coords.contains(south))
            {
                try next_coords.put(south, 1);
            }
        }
        // west
        if (coord.c >= 1) {
            const west = Coord{ .r = coord.r, .c = coord.c - 1 };
            if (coords.contains(west) and
                !visited.contains(west) and
                !next_coords.contains(west))
            {
                try next_coords.put(west, 1);
            }
        }
        try visited.put(coord, 1);
    }
    return 1;
}

pub fn count_contiguous_groups(
    allocator: std.mem.Allocator,
    coords: *const std.AutoHashMap(Coord, u8),
) !u32 {
    var count: u32 = 0;

    var visited = std.AutoHashMap(Coord, u8).init(allocator);
    defer visited.deinit();

    var coords_iterator = coords.keyIterator();
    while (coords_iterator.next()) |coord| {
        if (!visited.contains(coord.*)) {
            count += try find_contiguous_group_count(
                allocator,
                coord.*,
                coords,
                &visited,
            );
        }
    }

    return count;
}

pub fn count_contiguous_num_sides(
    allocator: std.mem.Allocator,
    map: *const Map,
    contiguous: *const std.ArrayList(Coord),
) !u32 {
    const cell_value = map.get(contiguous.items[0]).?;

    var north_facing = std.AutoHashMap(Coord, u8).init(allocator);
    var east_facing = std.AutoHashMap(Coord, u8).init(allocator);
    var south_facing = std.AutoHashMap(Coord, u8).init(allocator);
    var west_facing = std.AutoHashMap(Coord, u8).init(allocator);
    defer north_facing.deinit();
    defer east_facing.deinit();
    defer south_facing.deinit();
    defer west_facing.deinit();

    for (contiguous.items) |coord| {
        // north
        if (coord.r <= 0) {
            try north_facing.put(coord, 1);
        } else {
            const north = Coord{ .r = coord.r - 1, .c = coord.c };
            if (map.contains_coord(north) and cell_value != map.get(north).?) {
                try north_facing.put(coord, 1);
            }
        }
        // east
        if (coord.c >= map.width - 1) {
            try east_facing.put(coord, 1);
        } else {
            const east = Coord{ .r = coord.r, .c = coord.c + 1 };
            if (map.contains_coord(east) and cell_value != map.get(east).?) {
                try east_facing.put(coord, 1);
            }
        }
        // south
        if (coord.r >= map.height - 1) {
            try south_facing.put(coord, 1);
        } else {
            const south = Coord{ .r = coord.r + 1, .c = coord.c };
            if (map.contains_coord(south) and cell_value != map.get(south).?) {
                try south_facing.put(coord, 1);
            }
        }
        // west
        if (coord.c <= 0) {
            try west_facing.put(coord, 1);
        } else {
            const west = Coord{ .r = coord.r, .c = coord.c - 1 };
            if (map.contains_coord(west) and cell_value != map.get(west).?) {
                try west_facing.put(coord, 1);
            }
        }
    }

    const north_num_sides = try count_contiguous_groups(allocator, &north_facing);
    const east_num_sides = try count_contiguous_groups(allocator, &east_facing);
    const south_num_sides = try count_contiguous_groups(allocator, &south_facing);
    const west_num_sides = try count_contiguous_groups(allocator, &west_facing);

    return north_num_sides + east_num_sides + south_num_sides + west_num_sides;
}

// For part 2, total fence cost is the number of sides * area
pub fn find_p2_total_fence_cost(
    allocator: std.mem.Allocator,
    map: *const Map,
) !usize {
    var all_contiguous = std.ArrayList(std.ArrayList(Coord)).init(allocator);
    defer all_contiguous.deinit();

    try find_all_congtiguous_cells(
        allocator,
        map,
        &all_contiguous,
    );

    var total_fence_cost: usize = 0;
    for (all_contiguous.items) |contiguous| {
        const num_sides = try count_contiguous_num_sides(allocator, map, &contiguous);
        total_fence_cost += contiguous.items.len * num_sides;
    }

    for (all_contiguous.items) |cells| {
        cells.deinit();
    }

    return total_fence_cost;
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

    std.debug.print("Starting aoc 12: Garden Groups. input: '{s}'\n", .{args[1]});

    var map = try Map.init(allocator);
    defer map.deinit();
    try map.read_file(allocator, args[1]);
    //map.debug_print();

    const p1_total_fence_cost = try find_p1_total_fence_cost(allocator, &map);
    std.debug.print("p1_total_fence_cost: {d}\n", .{p1_total_fence_cost});

    const p2_total_fence_cost = try find_p2_total_fence_cost(allocator, &map);
    std.debug.print("p2_total_fence_cost: {d}\n", .{p2_total_fence_cost});
}
