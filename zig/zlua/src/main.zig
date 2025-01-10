const std = @import("std");
const repl = @import("./repl.zig");

pub fn main() !void {
    std.debug.print("Starting zlua repl...\n", .{});

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();

    //try repl.start_debug_tokenizer_repl(allocator, std.io.getStdIn().reader(), std.io.getStdOut().writer());
    try repl.start_debug_parser_repl(allocator, std.io.getStdIn().reader(), std.io.getStdOut().writer());

    std.debug.print("Exiting zlua...\n", .{});
}
