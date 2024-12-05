//! AOC day 3 solutions.
//! Run with the input file as the first parameter (relative path to where you run from).
//! Ex: `zig build run ./input`
//! Compiled on 0.14.0-dev
const std = @import("std");

pub fn read_char(readable: anytype) !?u8 {
    const char = readable.readByte() catch |err| switch (err) {
        error.EndOfStream => null,
        else => {
            std.log.err("Failed to open file: {s}", .{@errorName(err)});
            return err;
        },
    };
    return char;
}

pub fn read_valid_mul_start(readable: anytype, char: *?u8) !bool {
    if (char.* != null and char.*.? == 'm') {
        char.* = try read_char(readable);
        if (char.* != null and char.*.? == 'u') {
            char.* = try read_char(readable);
            if (char.* != null and char.*.? == 'l') {
                char.* = try read_char(readable);
                if (char.* != null and char.*.? == '(') {
                    return true;
                }
            }
        }
    }
    return false;
}

pub fn read_do_or_dont(readable: anytype, char: *?u8) !?bool {
    if (char.* != null and char.*.? == 'd') {
        char.* = try read_char(readable);
        if (char.* != null and char.*.? == 'o') {
            char.* = try read_char(readable);
            if (char.* != null and char.*.? == 'n') {
                char.* = try read_char(readable);
                if (char.* != null and char.*.? == '\'') {
                    char.* = try read_char(readable);
                    if (char.* != null and char.*.? == 't') {
                        char.* = try read_char(readable);
                        if (char.* != null and char.*.? == '(') {
                            char.* = try read_char(readable);
                            if (char.* != null and char.*.? == ')') {
                                return false;
                            }
                        }
                    }
                }
            } else if (char.* != null and char.*.? == '(') {
                char.* = try read_char(readable);
                if (char.* != null and char.*.? == ')') {
                    return true;
                }
            }
        }
    }
    return undefined;
}

pub fn read_int(readable: anytype, char: *?u8) !i32 {
    var buf: [32]u8 = undefined;
    var i: u8 = 0;
    char.* = try read_char(readable);
    while (char.* != null and '0' <= char.*.? and char.*.? <= '9') {
        buf[i] = char.*.?;
        i += 1;
        char.* = try read_char(readable);
    }
    const left_num: i32 = try std.fmt.parseInt(i32, buf[0..i], 10);
    return left_num;
}

pub fn mull_it_over(input_path: [:0]const u8) !i32 {
    var result: i32 = 0;

    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return err;
    };
    defer file.close();

    var buffered_reader = std.io.bufferedReader(file.reader());
    var char: ?u8 = try buffered_reader.reader().readByte();
    while (char != null) {
        if (try read_valid_mul_start(buffered_reader.reader(), &char)) {
            const left_num = try read_int(buffered_reader.reader(), &char);
            if (char != ',') {
                continue;
            }
            const right_num = try read_int(buffered_reader.reader(), &char);
            if (char != ')') {
                continue;
            }
            //std.debug.print("mul: left: {d}, right: {d}\n", .{ left_num, right_num });
            result += left_num * right_num;
        }
        char = try read_char(buffered_reader.reader());
    }

    return result;
}

pub fn mull_it_over_with_conditionals(input_path: [:0]const u8) !i32 {
    var result: i32 = 0;

    const file = std.fs.cwd().openFile(input_path, .{}) catch |err| {
        std.log.err("Failed to open file: {s}", .{@errorName(err)});
        return err;
    };
    defer file.close();

    var buffered_reader = std.io.bufferedReader(file.reader());
    var char: ?u8 = try buffered_reader.reader().readByte();
    var do_muls: bool = true;
    while (char != null) {
        const do_or_dont = try read_do_or_dont(buffered_reader.reader(), &char);
        if (do_or_dont != null) {
            do_muls = do_or_dont.?;
        }

        if (do_muls) {
            if (try read_valid_mul_start(buffered_reader.reader(), &char)) {
                const left_num = try read_int(buffered_reader.reader(), &char);
                if (char != ',') {
                    continue;
                }
                const right_num = try read_int(buffered_reader.reader(), &char);
                if (char != ')') {
                    continue;
                }
                //std.debug.print("mul: left: {d}, right: {d}\n", .{ left_num, right_num });
                result += left_num * right_num;
            }
        }
        char = try read_char(buffered_reader.reader());
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

    std.debug.print("Starting aoc 3: Mull It Over. input: '{s}'\n", .{args[1]});

    const mull_it_over_result = try mull_it_over(args[1]);
    std.debug.print("mull it over: {d}\n", .{mull_it_over_result});

    const mull_it_over_with_conditionals_result = try mull_it_over_with_conditionals(args[1]);
    std.debug.print("mull it over with conditionals: {d}\n", .{mull_it_over_with_conditionals_result});
}
