const std = @import("std");
//const Lexer = @import("./lexer.zig").Lexer;
const Tokenizer = @import("./tokenizer.zig").Tokenizer;
const Parser = @import("./parser.zig").Parser;

const PROMPT = ">> ";

var line_buf: [4096:0]u8 = undefined;

// REPL that stops after tokenizing and dumps the tokenized input to stdout.
pub fn start_debug_tokenizer_repl(allocator: std.mem.Allocator, reader: anytype, writer: anytype) !void {
    while (true) {
        writer.print("{s}", .{PROMPT}) catch unreachable;

        const line = (reader.readUntilDelimiterOrEof(&line_buf, '\n') catch unreachable) orelse break;
        // The "[:0]u8" type is zig's "sentinel terminate" (null terminate like c).
        // The tokenizer relies on an ending null byte to make walking the text simpler.
        const line_null_terminated: [:0]u8 = try std.fmt.allocPrintZ(allocator, "{s}", .{line});
        defer allocator.free(line_null_terminated);

        var tokenizer = Tokenizer.init(allocator, line_null_terminated);
        defer tokenizer.deinit();

        while (true) {
            const token = tokenizer.next();
            if (token.tag == .invalid) {
                writer.print("Invalid token!\n", .{}) catch unreachable;
                break;
            } else if (token.tag == .eof) {
                break;
            }
            writer.print("{any}\n", .{token}) catch unreachable;
        }
    }
}

// REPL that stops after parsing and creating the ast and dumps the parsed ast to stdout.
pub fn start_debug_parser_repl(allocator: std.mem.Allocator, reader: anytype, writer: anytype) !void {
    while (true) {
        writer.print("{s}", .{PROMPT}) catch unreachable;

        var line = (reader.readUntilDelimiterOrEof(&line_buf, '\n') catch unreachable) orelse break;
        line = line[0 .. line.len - 1]; // remove "\r\n"
        const line_null_terminated: [:0]u8 = try std.fmt.allocPrintZ(allocator, "{s}", .{line});
        var tokenizer = Tokenizer.init(allocator, line_null_terminated);
        defer tokenizer.deinit();
        var parser = try Parser.init(&tokenizer, allocator);
        const program = try parser.parseProgram();
        std.debug.print("{}", .{program});
    }
}

// TODO: full evaluator repl
