//! AOC day 7 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

pub const Calculation = struct {
    result: u64,
    operands: std.ArrayList(u64),

    pub fn init(allocator: std.mem.Allocator) Calculation {
        return .{
            .result = 0,
            .operands = std.ArrayList(u64).init(allocator),
        };
    }

    pub fn print(self: *Calculation) void{
        std.debug.print("{d}: ", .{self.result});
        for (self.operands.items) |operand| {
            std.debug.print("{d}, ", .{operand});
        }
        std.debug.print("\n", .{});
    }

    pub fn deinit(self: *Calculation) void {
        self.operands.deinit();
    }
};

pub fn read_input(
    allocator: std.mem.Allocator,
    input_path: [:0]const u8,
    calculations: *std.ArrayList(Calculation),
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

        var calculation = Calculation.init(allocator);
        
        var i: u32 = 0;
        var left: u32 = 0;
        var right: u32 = 0;
        while (i < line.len) {
            if (line[i] == ':') {
                calculation.result = try std.fmt.parseInt(u64, line[0..i], 10);
                i += 2;
                left = i;
                continue;
            } else if (line[i] == ' ') {
                right = i;
                const num = try std.fmt.parseInt(u64, line[left..right], 10);
                try calculation.operands.append(num);
                left = right + 1;
            }
            i += 1;
        }
        right = i;
        const num = try std.fmt.parseInt(u64, line[left..right], 10);
        try calculation.operands.append(num);
        try calculations.append(calculation);
    }
}

pub fn is_calculation_valid(
    allocator: std.mem.Allocator,
    calculation: *Calculation,
) !bool {
    var next_queue = std.ArrayList(u64).init(allocator);
    defer next_queue.deinit();
    try next_queue.append(calculation.*.operands.items[0]);
    
    for (1..calculation.*.operands.items.len) |i| {
        var prev_queue = try next_queue.clone();
        next_queue.clearAndFree();
        defer prev_queue.deinit();

        while (prev_queue.items.len > 0) {
            const current: u64 = prev_queue.pop();
            const add = current + calculation.*.operands.items[i];
            try next_queue.append(add);

            const mul = current * calculation.*.operands.items[i];
            try next_queue.append(mul);
        }
    }

    //std.debug.print("leaf nodes: {d}\n", .{next_queue.items.len});
    for (next_queue.items) |num| {
        //std.debug.print("   {d}\n", .{node.*.num});
        if (num == calculation.result) {
            return true;
        }
    }
    return false;
}

/// strategy
/// * build a binary tree with each left=+ and right=* and depth being number of operands
/// * walk tree and build strings of permutation operation orders.
/// * execute those permutation operation orders.
/// (ended up just using stacks, didn't need the entire trees, but they might have been nice for visualization)
pub fn get_accum_valid_calculations(
    allocator: std.mem.Allocator,
    calculations: std.ArrayList(Calculation),
) !u64 {
    var result: u64 = 0;
    for (calculations.items) |*calculation| {
        if (try is_calculation_valid(allocator, calculation)) {
            result = result + @as(u64, @intCast(calculation.result));
        }
    }
    return result;
}

pub fn is_p2_calculation_valid(
    allocator: std.mem.Allocator,
    calculation: *Calculation,
) !bool {
    var next_queue = std.ArrayList(u64).init(allocator);
    defer next_queue.deinit();
    try next_queue.append(calculation.*.operands.items[0]);
    
    for (1..calculation.*.operands.items.len) |i| {
        var prev_queue = try next_queue.clone();
        next_queue.clearAndFree();
        defer prev_queue.deinit();

        while (prev_queue.items.len > 0) {
            const current: u64 = prev_queue.pop();
            const add = current + calculation.*.operands.items[i];
            try next_queue.append(add);

            const mul = current * calculation.*.operands.items[i];
            try next_queue.append(mul);

            var left_buffer: [128]u8 = undefined;
            const left_slice = try std.fmt.bufPrint(&left_buffer, "{d}", .{current});
            var right_buffer: [128]u8 = undefined;
            const right_slice = try std.fmt.bufPrint(&right_buffer, "{d}", .{calculation.*.operands.items[i]});
            var concat_string: [256]u8 = undefined;
            std.mem.copyForwards(u8, concat_string[0..], left_slice);
            std.mem.copyForwards(u8, concat_string[left_slice.len..], right_slice);            

            const concat = try std.fmt.parseInt(u64, concat_string[0..left_slice.len + right_slice.len], 10);

            try next_queue.append(concat);
        }
    }

    //std.debug.print("leaf nodes: {d}\n", .{next_queue.items.len});
    for (next_queue.items) |num| {
        //std.debug.print("   {d}\n", .{node.*.num});
        if (num == calculation.result) {
            return true;
        }
    }
    return false;
}

pub fn get_p2_accum_valid_calculations(
    allocator: std.mem.Allocator,
    calculations: std.ArrayList(Calculation),
) !u64 {
    var result: u64 = 0;
    for (calculations.items) |*calculation| {
        if (try is_p2_calculation_valid(allocator, calculation)) {
            result = result + @as(u64, @intCast(calculation.result));
        }
    }
    return result;
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

    std.debug.print("Starting aoc 7: bridge repair. input: '{s}'\n", .{args[1]});

    var calculations = std.ArrayList(Calculation).init(allocator);
    defer calculations.deinit();

    try read_input(allocator, args[1], &calculations);

    const accum_valid_calcs = try get_accum_valid_calculations(allocator, calculations);
    std.debug.print("Part 1: accum valid calcs: {d}\n", .{accum_valid_calcs});

    const p2_accum = try get_p2_accum_valid_calculations(allocator, calculations);
    std.debug.print("Part 2: accum valid calcs: {d}\n", .{p2_accum});

    for (calculations.items) |*calculation| {
        calculation.deinit();
    }
}
