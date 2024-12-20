//! AOC day 9 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

const FileBlock = struct {
    id: u32,
};

pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    disk_map: *std.ArrayList(FileBlock),
) !void {
    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return;
    };
    defer file.close();

    const delimiter = '\n';
    const max_size = std.math.maxInt(usize);
    // I should really use a buffered chunk reader approach for this but the last 8 days have all been line based and I'm feeling a bit lazy.
    while (file.reader().readUntilDelimiterOrEofAlloc(allocator, delimiter, max_size) catch |err| {
        std.log.err("Failed to read line: {s}", .{@errorName(err)});
        return;
    }) |line| {
        defer allocator.free(line);
        if (line.len == 0) {
            break; // skip empty lines
        }

        var i: u32 = 0;
        var file_id: u32 = 1;
        while (i < line.len) {
            const size: u8 = line[i] - '0';
            if (i % 2 == 0) { // every other num is either a file or empty space
                for (0..size) |j| {
                    _ = j;
                    try disk_map.append(FileBlock{ .id = file_id });
                }
                file_id += 1;
            } else {
                for (0..size) |j| {
                    _ = j;
                    try disk_map.append(FileBlock{ .id = 0 });
                }
            }
            i += 1;
        }
    }
}

pub fn print_file_map(file_map: std.ArrayList(FileBlock)) void {
    std.debug.print("map: ", .{});
    for (file_map.items) |file_node| {
        if (file_node.id == 0) {
            std.debug.print(".", .{});
        } else {
            std.debug.print("{d}", .{file_node.id - 1});
        }
    }
    std.debug.print("\n", .{});
}

pub fn find_next_empty(file_map: *std.ArrayList(FileBlock), start: usize) usize {
    for (start..file_map.items.len) |i| {
        if (file_map.items[i].id == 0) {
            return i;
        }
    }
    return file_map.items.len - 1;
}

pub fn find_rightmost_file_block(file_map: *std.ArrayList(FileBlock), right_start: usize) usize {
    var i: usize = right_start;
    while (i > 0) {
        if (file_map.items[i].id != 0) {
            return i;
        }
        i -= 1;
    }
    return 0;
}

pub fn condense_blocks(file_map: *std.ArrayList(FileBlock)) void {
    var left: usize = find_next_empty(file_map, 0);
    var right: usize = find_rightmost_file_block(file_map, file_map.items.len - 1);

    while (left < right) {
        file_map.items[left].id = file_map.items[right].id;
        file_map.items[right].id = 0;
        left = find_next_empty(file_map, left);
        right = find_rightmost_file_block(file_map, right);
    }
}

pub fn calculate_checksum(file_map: std.ArrayList(FileBlock)) usize {
    var result: usize = 0;
    for (file_map.items, 0..) |file_block, i| {
        if (file_block.id != 0) {
            result += (file_block.id - 1) * i;
        }
    }
    return result;
}

pub fn find_file_start_and_size(
    file_map: *std.ArrayList(FileBlock),
    file_id: u32,
    right_start: usize,
) [2]usize {
    var i: usize = right_start;
    while (i > 0) {
        const cur_file_id = file_map.items[i].id;
        if (file_id == cur_file_id) {
            var file_size: usize = 0;
            if (cur_file_id != 0) {
                var j: usize = i;
                while (j > 0) {
                    if (file_map.items[j].id != cur_file_id) {
                        return [2]usize{ j + 1, file_size };
                    }
                    file_size += 1;
                    j -= 1;
                }
            }
        }
        i -= 1;
    }
    return [2]usize{ 0, 0 };
}

pub fn find_next_empty_min_size(
    file_map: *std.ArrayList(FileBlock),
    start: usize,
    min_size: usize,
    max_r: usize,
) usize {
    var cur_size: usize = 0;
    for (start..file_map.items.len) |i| {
        if (i >= max_r) {
            return file_map.items.len - 1;
        }
        if (file_map.items[i].id == 0) {
            cur_size += 1;
            if (cur_size >= min_size) {
                return i - cur_size + 1; // return the leftmost address
            }
        } else {
            cur_size = 0;
        }
    }
    return file_map.items.len - 1;
}

pub fn condense_with_defrag(file_map: *std.ArrayList(FileBlock)) void {
    // Find the rightmost file id.
    var file_id: u32 = 0;
    var i: usize = file_map.items.len - 1;
    while (i > 0) {
        if (file_map.items[i].id != 0) {
            file_id = file_map.items[i].id;
            break;
        }
        i -= 1;
    }

    // Find the start index and size of the rightmost file
    var right_result: [2]usize = find_file_start_and_size(file_map, file_id, file_map.items.len - 1);
    // Find the leftmost freespace the file could fit into
    var left: usize = find_next_empty_min_size(file_map, 0, right_result[1], i);

    while (file_id > 1) {
        if (left < file_map.items.len - 1 and right_result[0] > 0) {
            var j: usize = 0;
            while (j < right_result[1]) {
                file_map.items[left + j].id = file_map.items[right_result[0] + j].id;
                file_map.items[right_result[0] + j].id = 0;
                j += 1;
            }
        }

        file_id -= 1;
        right_result = find_file_start_and_size(file_map, file_id, right_result[0]);
        left = find_next_empty_min_size(file_map, 0, right_result[1], right_result[0]);
    }
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

    std.debug.print("Starting aoc 9: Disk Fragmenter. input: '{s}'\n", .{args[1]});

    var file_map = std.ArrayList(FileBlock).init(allocator);
    var file_map2 = std.ArrayList(FileBlock).init(allocator);
    defer file_map.deinit();
    defer file_map2.deinit();
    try read_input(allocator, args[1], &file_map);
    try read_input(allocator, args[1], &file_map2);

    //print_file_map(file_map);
    condense_blocks(&file_map);
    const checksum: usize = calculate_checksum(file_map);
    std.debug.print("checksum: {d}\n", .{checksum});

    //print_file_map(file_map2);
    condense_with_defrag(&file_map2);
    const checksum2: usize = calculate_checksum(file_map2);
    std.debug.print("checksum2: {d}\n", .{checksum2});
}
