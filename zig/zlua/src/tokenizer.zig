//!
//! Responsible for Tokenizing/Lexing (performing lexigraphical analysis on) a source
//! code text file and generating individual lexical "tokens".
//!
//! Lua tokens:
//! https://www.lua.org/manual/5.4/manual.html#3.1
//!
//! Lua full grammar:
//! https://www.lua.org/manual/5.4/manual.html#9
//!
//! Notes:
//! * Architecture/organization wise, inspired by the "aro" c compiler in the
//! zig repo (https://github.com/ziglang/zig/blob/master/lib/compiler/aro/aro/Tokenizer.zig).
//!

const std = @import("std");

//
// An individual "token" of information in a source code text file along with some tracked
// meta-data around the token for diagnostic purposes.
//
pub const Token = struct {
    tag: Tag,
    location: Location,

    // Location (start and end character indices) of the token in the source text.
    pub const Location = struct {
        start: usize,
        end: usize,
    };

    /// Retrieves the source text this token was generated from.
    pub fn getValue(self: Token, source: [:0]const u8) []const u8 {
        return source[self.location.start..self.location.end];
    }

    // Keywords in the Lua programming language.
    // https://www.lua.org/manual/5.4/
    pub const keywords = std.StaticStringMap(Tag).initComptime(.{
        .{ "and", .keyword_and },
        .{ "break", .keyword_break },
        .{ "do", .keyword_do },
        .{ "else", .keyword_else },
        .{ "elseif", .keyword_elseif },
        .{ "end", .keyword_end },
        .{ "false", .keyword_false },
        .{ "for", .keyword_for },
        .{ "function", .keyword_function },
        .{ "goto", .keyword_goto },
        .{ "if", .keyword_if },
        .{ "in", .keyword_in },
        .{ "local", .keyword_local },
        .{ "nil", .keyword_nil },
        .{ "not", .keyword_not },
        .{ "or", .keyword_or },
        .{ "repeat", .keyword_repeat },
        .{ "return", .keyword_return },
        .{ "then", .keyword_then },
        .{ "true", .keyword_true },
        .{ "until", .keyword_until },
        .{ "while", .keyword_while },
    });

    pub const Tag = enum {
        invalid,
        eof, // 0x04 (end-of-file)

        identifier, // text
        string_literal, // "text" OR 'text'
        multiline_string_literal,
        number_literal, // 1234
        label, // ::text::

        equal, // =
        plus, // +
        dash, // -
        dash_dash, // --
        asterisk, // *
        slash, // /
        slash_slash, // //
        modulo, // %
        caret, // ^
        bang, // !
        hash, // #
        equal_equal, // ==
        not_equal, // != or ~=
        dot, // .
        dot_dot, // ..
        dot_dot_dot, // ...
        comma, // ,
        colon, // :
        semicolon, // ;
        parentheses_left, // (
        parentheses_right, // )
        square_bracket_left, // [
        square_bracket_right, // ]
        curly_bracket_left, // {
        curly_bracket_right, // }
        angle_bracket_left, // <
        angle_bracket_right, // >
        angle_bracket_left_equal, // <=
        angle_bracket_right_equal, // >=

        // Keywords
        keyword_and,
        keyword_break,
        keyword_do,
        keyword_else,
        keyword_elseif,
        keyword_end,
        keyword_false,
        keyword_for,
        keyword_function,
        keyword_goto,
        keyword_if,
        keyword_in,
        keyword_local,
        keyword_nil,
        keyword_not,
        keyword_or,
        keyword_repeat,
        keyword_return,
        keyword_then,
        keyword_true,
        keyword_until,
        keyword_while,

        pub fn format(self: Tag, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
            try writer.print("{s}", .{std.enums.tagName(Tag, self).?});
        }
    };

    pub fn getKeyword(bytes: []const u8) ?Tag {
        return keywords.get(bytes);
    }
};

pub const TokenizerError = error{OutOfMemory} || error{
    UnfinishedString,
    MalformedNumberLiteral,
};

// Creates tokens from raw source code file character data.
pub const Tokenizer = struct {
    // Memory allocator used for any dynamic memory operations.
    allocator: std.mem.Allocator,
    // Source text buffer being tokenized/lexed.
    buffer: [:0]const u8,
    // Current index in the source text buffer, used to process the next token.
    index: usize,
    // Array noting the index of each new line character, used to look up which line
    // number a token is on.
    new_lines: std.ArrayList(usize),

    /// Initializes a new "Tokenizer".
    pub fn init(allocator: std.mem.Allocator, buffer: [:0]const u8) Tokenizer {
        return .{
            .allocator = allocator,
            .buffer = buffer,
            .index = 0,
            .new_lines = std.ArrayList(usize).init(allocator),
            //.index = if (std.mem.startsWith(u8, buffer, "\xEF\xBB\xBF")) 3 else 0,
        };
    }

    // De-initializes any memory allocated by the tokenizer.
    pub fn deinit(self: *Tokenizer) void {
        self.new_lines.deinit();
    }

    /// Reads a single character from the tokenizer with bounds checks.
    pub fn readChar(self: *Tokenizer) void {
        if (self.read_position >= self.input.len) {
            self.char = 0;
        } else {
            self.char = self.input[self.read_position];
        }
        self.position = self.read_position;
        self.read_position += 1;
    }

    /// Reads the next token from the source text file.
    pub fn next(self: *Tokenizer) TokenizerError!Token {
        try self.readWhitespace();
        var token: Token = .{
            .tag = undefined,
            .location = .{
                .start = self.index,
                .end = undefined,
            },
        };
        if (self.index >= self.buffer.len) {
            token.tag = .eof;
            token.location.end = self.index;
            return token;
        }
        switch (self.buffer[self.index]) {
            0 => {},
            0x04 => {
                token.tag = .eof;
                self.index += 1;
            },
            '=' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '=' => {
                        token.tag = .equal_equal;
                        self.index += 1;
                    },
                    else => {
                        token.tag = .equal;
                    },
                }
            },
            '+' => {
                self.index += 1;
                token.tag = .plus;
            },
            '-' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '-' => {
                        token.tag = .dash_dash;
                        self.index += 1;
                    },
                    // TODO: multi-line comment --[[
                    else => {
                        token.tag = .dash;
                    },
                }
            },
            '*' => {
                self.index += 1;
                token.tag = .asterisk;
            },
            '/' => {
                // TODO: double slash for floor division?
                // slash_slash
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '/' => {
                        token.tag = .slash_slash;
                        self.index += 1;
                    },
                    else => {
                        token.tag = .slash;
                    },
                }
            },
            '%' => {
                self.index += 1;
                token.tag = .modulo;
            },
            '^' => {
                self.index += 1;
                token.tag = .caret;
            },
            '!' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '=' => {
                        token.tag = .not_equal;
                        self.index += 1;
                    },
                    else => token.tag = .bang,
                }
            },
            '~' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '=' => {
                        token.tag = .not_equal;
                        self.index += 1;
                    },
                    else => token.tag = .invalid,
                }
            },
            '.' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '.' => {
                        self.index += 1;
                        switch (self.buffer[self.index]) {
                            '.' => {
                                token.tag = .dot_dot_dot;
                                self.index += 1;
                            },
                            else => token.tag = .dot_dot,
                        }
                    },
                    else => token.tag = .dot,
                }
            },
            ',' => {
                self.index += 1;
                token.tag = .comma;
            },
            ':' => {
                self.index += 1;
                token.tag = .colon;
            },
            ';' => {
                self.index += 1;
                token.tag = .semicolon;
            },
            '(' => {
                self.index += 1;
                token.tag = .parentheses_left;
            },
            ')' => {
                self.index += 1;
                token.tag = .parentheses_right;
            },
            '[' => {
                // TODO: long strings "[[ string ]]" or "[=[ string ]=]" or "[==[ string ]==]"
                // supports arbitrary number of = signs, closing must match with same number
                self.index += 1;
                token.tag = .square_bracket_left;
            },
            ']' => {
                self.index += 1;
                token.tag = .square_bracket_right;
            },
            '{' => {
                self.index += 1;
                token.tag = .curly_bracket_left;
            },
            '}' => {
                self.index += 1;
                token.tag = .curly_bracket_right;
            },
            '<' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '=' => {
                        token.tag = .angle_bracket_left_equal;
                        self.index += 1;
                    },
                    else => token.tag = .angle_bracket_left,
                }
            },
            '>' => {
                self.index += 1;
                switch (self.buffer[self.index]) {
                    '=' => {
                        token.tag = .angle_bracket_right_equal;
                        self.index += 1;
                    },
                    else => token.tag = .angle_bracket_right,
                }
            },
            '"', '\'' => {
                const string_start_char = self.buffer[self.index];
                token.location.start = self.index;
                self.index += 1;
                while (true) {
                    if (self.index >= self.buffer.len) {
                        token.tag = .invalid;
                        return TokenizerError.UnfinishedString;
                    }
                    if (self.buffer[self.index] == string_start_char) {
                        self.index += 1;
                        token.tag = .string_literal;
                        break;
                    } else {
                        self.index += 1;
                    }
                }
            },
            '0'...'9' => {
                if (self.buffer[self.index] == '0' and
                    self.index + 1 < self.buffer.len and
                    self.buffer[self.index + 1] == 'x')
                {
                    // TODO: parse hexadecimal
                    self.index += 2;
                    while (isHexDigit(self.buffer[self.index])) {
                        self.index += 1;
                    }
                } else { // normal integer "12314"
                    // TODO: check for invalid token "123my_thing"
                    // TODO: check for floats/decimals 12.34
                    self.index += 1;
                    while (isDigit(self.buffer[self.index])) {
                        self.index += 1;
                    }
                }
                // We need a space or close paren, or something between numbers and other tokens.
                // ex: handle `1234my_number` which should be `1234 my_number`.
                if (isStartingIdentifierCharacter(self.buffer[self.index])) {
                    return TokenizerError.MalformedNumberLiteral;
                }
                // if (self.index < self.buffer.len - 1 and !isWhitespace(self.buffer[self.index])) {
                //     return TokenizerError.MalformedNumberLiteral;
                // }
                token.tag = .number_literal;
            },
            else => {
                if (isStartingIdentifierCharacter(self.buffer[self.index])) {
                    while (isContinuingIdentifierCharacter(self.buffer[self.index])) {
                        self.index += 1;
                    }
                    const identifier = self.buffer[token.location.start..self.index];
                    if (Token.getKeyword(identifier)) |tag| {
                        token.tag = tag;
                    } else {
                        token.tag = .identifier;
                    }
                } else {
                    // TODO: invalid token?
                }
            },
        }
        token.location.end = self.index;
        return token;
    }

    /// Reads any whitespace characters, keeping track of newlines for diagnostic purposes.
    pub fn readWhitespace(self: *Tokenizer) TokenizerError!void {
        while (isWhitespace(self.buffer[self.index]) == true) // '\f' - not in zig, same as above
        {
            if (self.buffer[self.index] == '\n') {
                try self.new_lines.append(self.index);
            }
            self.index += 1;
        }
    }

    /// Determines if the character is a "white space" character or not (most white
    /// space is not significant, but you need at least 1 between most tokens).
    ///
    /// https://www.lua.org/manual/5.4/manual.html#3.1
    /// Lua recognizes whitespace as "standard ASCII whitespace characters":
    /// * space (' ')
    /// * form feed ('\f') - (0x0c) not in zig, low priority bug - https://github.com/ziglang/zig/issues/21564)
    /// * new line ('\n')
    /// * carriage return ('\r')
    /// * horizontal tab ('\t')
    /// * vertical tab ('\v') - (0x0b) not in zig, same as above
    pub fn isWhitespace(char: u8) bool {
        return switch (char) {
            ' ', '\t', '\r', '\n', 0x0b, 0x0c => true,
            else => false,
        };
    }

    /// Determines if the character is valid to start a new name/identifier.
    /// Names can't start with numbers.
    /// https://www.lua.org/manual/5.4/manual.html#3.1
    /// TODO: support unicode?
    pub fn isStartingIdentifierCharacter(char: u8) bool {
        return switch (char) {
            'a'...'z', 'A'...'Z', '_' => true,
            else => false,
        };
    }

    /// Determines if the character is a valid non-first name/identifier character.
    /// Non-first numbers can be numbers.
    /// TODO: support unicode?
    pub fn isContinuingIdentifierCharacter(char: u8) bool {
        return switch (char) {
            'a'...'z', 'A'...'Z', '0'...'9', '_' => true,
            else => false,
        };
    }

    /// TODO: revisit number handling.
    /// Not sure how to handle numbers yet. LUA numbers are floats but int support has been
    /// added under the hood at some point for performance. need to support both?
    pub fn isDigit(char: u8) bool {
        return switch (char) {
            '0'...'9' => true,
            else => false,
        };
    }

    pub fn isHexDigit(char: u8) bool {
        return switch (char) {
            '0'...'9', 'a'...'f', 'A'...'F' => true,
            else => false,
        };
    }

    pub fn isValidPostDigitCharacter(char: u8) bool {
        return switch (char) {
            'a'...'z', 'A'...'Z' => false,
            else => true,
        };
    }

    /// Retrieves the line number the token is on.
    pub fn getLine(self: Tokenizer, token: Token) usize {
        if (self.new_lines.items.len == 0) {
            return 1;
        }
        if (token.location.start >= self.new_lines.getLast()) {
            return self.new_lines.items.len + 1;
        }
        // binary search
        var low: usize = 0;
        var high: usize = self.new_lines.items.len - 1;
        while (low < high) {
            const mid: usize = low + ((high - low) / 2);
            if (self.new_lines.items[mid] > token.location.start) {
                high = mid - 1;
            } else if (self.new_lines.items[mid] < token.location.start) {
                low = mid + 1;
            } else {
                return mid;
            }
        }
        return 1;
    }
};

// All code below this point is only used for testing.

//
// MARK: Test Helpers
//
fn expectToken(expected: Token, actual: Token) !void {
    try std.testing.expectEqual(@intFromEnum(expected), @intFromEnum(actual));
    if (@intFromEnum(expected) == @intFromEnum(actual)) {
        switch (actual) {
            .identifier => try std.testing.expectEqualStrings(expected.identifier, actual.identifier),
            .string => try std.testing.expectEqualStrings(expected.string, actual.string),
            .integer => try std.testing.expectEqualStrings(expected.integer, actual.integer),
            .label => try std.testing.expectEqualStrings(expected.label, actual.label),
            else => {},
        }
    }
}

//
// MARK: TESTS
//
test "next individual_tokens" {
    const TestCase = struct { input: [:0]const u8, expected: Token.Tag };
    const tests = [_]TestCase{
        //Test{ .input = "", .expected = .invalid }, TODO: what should this return? error?
        .{ .input = "\x04", .expected = .eof },
        .{ .input = "=", .expected = .equal },
        .{ .input = "==", .expected = .equal_equal },
        .{ .input = "+", .expected = .plus },
        .{ .input = "-", .expected = .dash },
        .{ .input = "--", .expected = .dash_dash },
        .{ .input = "*", .expected = .asterisk },
        .{ .input = "/", .expected = .slash },
        .{ .input = "//", .expected = .slash_slash },
        .{ .input = "%", .expected = .modulo },
        .{ .input = "^", .expected = .caret },
        .{ .input = "!", .expected = .bang },
        .{ .input = "!=", .expected = .not_equal },
        .{ .input = "~=", .expected = .not_equal },
        .{ .input = "<", .expected = .angle_bracket_left },
        .{ .input = ">", .expected = .angle_bracket_right },
        .{ .input = "<=", .expected = .angle_bracket_left_equal },
        .{ .input = ">=", .expected = .angle_bracket_right_equal },
        .{ .input = "(", .expected = .parentheses_left },
        .{ .input = ")", .expected = .parentheses_right },
        .{ .input = "[", .expected = .square_bracket_left },
        .{ .input = "]", .expected = .square_bracket_right },
        .{ .input = "{", .expected = .curly_bracket_left },
        .{ .input = "}", .expected = .curly_bracket_right },
        .{ .input = ".", .expected = .dot },
        .{ .input = "..", .expected = .dot_dot },
        .{ .input = "...", .expected = .dot_dot_dot },
        .{ .input = ",", .expected = .comma },
        .{ .input = ":", .expected = .colon },
        .{ .input = ";", .expected = .semicolon },
    };

    for (tests) |testCase| {
        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        const token = try tokenizer.next();
        try std.testing.expectEqual(testCase.expected, token.tag);
        try std.testing.expectEqual(0, token.location.start);
        try std.testing.expectEqual(testCase.input.len, token.location.end);
    }
}

test "next keywords" {
    const TestCase = struct { input: [:0]const u8, expected: Token.Tag };
    const tests = [_]TestCase{
        .{ .input = "and", .expected = .keyword_and },
        .{ .input = "break", .expected = .keyword_break },
        .{ .input = "do", .expected = .keyword_do },
        .{ .input = "else", .expected = .keyword_else },
        .{ .input = "elseif", .expected = .keyword_elseif },
        .{ .input = "end", .expected = .keyword_end },
        .{ .input = "false", .expected = .keyword_false },
        .{ .input = "for", .expected = .keyword_for },
        .{ .input = "function", .expected = .keyword_function },
        .{ .input = "if", .expected = .keyword_if },
        .{ .input = "in", .expected = .keyword_in },
        .{ .input = "local", .expected = .keyword_local },
        .{ .input = "nil", .expected = .keyword_nil },
        .{ .input = "not", .expected = .keyword_not },
        .{ .input = "or", .expected = .keyword_or },
        .{ .input = "repeat", .expected = .keyword_repeat },
        .{ .input = "return", .expected = .keyword_return },
        .{ .input = "then", .expected = .keyword_then },
        .{ .input = "true", .expected = .keyword_true },
        .{ .input = "until", .expected = .keyword_until },
        .{ .input = "while", .expected = .keyword_while },
    };

    for (tests) |t| {
        var tokenizer = Tokenizer.init(std.testing.allocator, t.input);
        defer tokenizer.deinit();
        const token = try tokenizer.next();
        try std.testing.expectEqual(t.expected, token.tag);
        try std.testing.expectEqual(0, token.location.start);
        try std.testing.expectEqual(t.input.len, token.location.end);
    }
}

test "next HelloWorld" {
    const input = "print \"Hello World\"";
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    defer tokenizer.deinit();

    // print
    const print_token = try tokenizer.next();
    try std.testing.expectEqual(.identifier, print_token.tag);
    try std.testing.expectEqualStrings("print", print_token.getValue(input));

    // "Hello World"
    const string_literal_token = try tokenizer.next();
    try std.testing.expectEqual(.string_literal, string_literal_token.tag);
    try std.testing.expectEqualStrings("\"Hello World\"", string_literal_token.getValue(input));
}

test "next simple_snippets" {
    const input =
        \\print "Hello World"
        \\
        \\print(8 * 9)
        \\
        \\function add (a)
        \\  local sum = 0
        \\  for i,v in ipairs(a) do
        \\    sum = sum + v
        \\  end
        \\  return sum
        \\end
        \\
        \\if x == 1 then
        \\  print(true);
        \\end
        \\
    ;

    const TestCase = struct {
        tag: Token.Tag,
        text: [:0]const u8,
        line: usize,
    };
    const expected_tokens = [_]TestCase{
        .{ .tag = .identifier, .text = "print", .line = 1 },
        .{ .tag = .string_literal, .text = "\"Hello World\"", .line = 1 },

        .{ .tag = .identifier, .text = "print", .line = 3 },
        .{ .tag = .parentheses_left, .text = "(", .line = 3 },
        .{ .tag = .number_literal, .text = "8", .line = 3 },
        .{ .tag = .asterisk, .text = "*", .line = 3 },
        .{ .tag = .number_literal, .text = "9", .line = 3 },
        .{ .tag = .parentheses_right, .text = ")", .line = 3 },

        .{ .tag = .keyword_function, .text = "function", .line = 5 },
        .{ .tag = .identifier, .text = "add", .line = 5 },
        .{ .tag = .parentheses_left, .text = "(", .line = 5 },
        .{ .tag = .identifier, .text = "a", .line = 5 },
        .{ .tag = .parentheses_right, .text = ")", .line = 5 },

        .{ .tag = .keyword_local, .text = "local", .line = 6 },
        .{ .tag = .identifier, .text = "sum", .line = 6 },
        .{ .tag = .equal, .text = "=", .line = 6 },
        .{ .tag = .number_literal, .text = "0", .line = 6 },

        .{ .tag = .keyword_for, .text = "for", .line = 7 },
        .{ .tag = .identifier, .text = "i", .line = 7 },
        .{ .tag = .comma, .text = ",", .line = 7 },
        .{ .tag = .identifier, .text = "v", .line = 7 },
        .{ .tag = .keyword_in, .text = "in", .line = 7 },
        .{ .tag = .identifier, .text = "ipairs", .line = 7 },
        .{ .tag = .parentheses_left, .text = "(", .line = 7 },
        .{ .tag = .identifier, .text = "a", .line = 7 },
        .{ .tag = .parentheses_right, .text = ")", .line = 7 },
        .{ .tag = .keyword_do, .text = "do", .line = 7 },

        .{ .tag = .identifier, .text = "sum", .line = 8 },
        .{ .tag = .equal, .text = "=", .line = 8 },
        .{ .tag = .identifier, .text = "sum", .line = 8 },
        .{ .tag = .plus, .text = "+", .line = 8 },
        .{ .tag = .identifier, .text = "v", .line = 8 },

        .{ .tag = .keyword_end, .text = "end", .line = 9 },
        .{ .tag = .keyword_return, .text = "return", .line = 10 },
        .{ .tag = .identifier, .text = "sum", .line = 10 },
        .{ .tag = .keyword_end, .text = "end", .line = 11 },

        .{ .tag = .keyword_if, .text = "if", .line = 13 },
        .{ .tag = .identifier, .text = "x", .line = 13 },
        .{ .tag = .equal_equal, .text = "==", .line = 13 },
        .{ .tag = .number_literal, .text = "1", .line = 13 },
        .{ .tag = .keyword_then, .text = "then", .line = 13 },
        .{ .tag = .identifier, .text = "print", .line = 14 },
        .{ .tag = .parentheses_left, .text = "(", .line = 14 },
        .{ .tag = .keyword_true, .text = "true", .line = 14 },
        .{ .tag = .parentheses_right, .text = ")", .line = 14 },
        .{ .tag = .semicolon, .text = ";", .line = 14 },
        .{ .tag = .keyword_end, .text = "end", .line = 15 },
    };

    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    defer tokenizer.deinit();
    for (expected_tokens) |expected| {
        const token = try tokenizer.next();
        try std.testing.expectEqual(expected.tag, token.tag);
        try std.testing.expectEqualStrings(expected.text, tokenizer.buffer[token.location.start..token.location.end]);
        try std.testing.expectEqual(expected.line, tokenizer.getLine(token));
    }
}

test "next unfinished string" {
    const input = "print 'hello world;"; // no closing '
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    defer tokenizer.deinit();
    _ = try tokenizer.next(); // skip "print"
    try std.testing.expectError(TokenizerError.UnfinishedString, tokenizer.next());
}

test "next number_literals" {
    const TestCase = struct { input: [:0]const u8, expected: Token.Tag };
    const tests = [_]TestCase{
        .{ .input = "0", .expected = .number_literal },
        .{ .input = "1", .expected = .number_literal },
        .{ .input = "01", .expected = .number_literal },
        .{ .input = "10", .expected = .number_literal },
        .{ .input = "1234", .expected = .number_literal },
        .{ .input = "0x1234", .expected = .number_literal },
    };

    for (tests) |t| {
        var tokenizer = Tokenizer.init(std.testing.allocator, t.input);
        defer tokenizer.deinit();
        const token = try tokenizer.next();
        try std.testing.expectEqual(t.expected, token.tag);
        try std.testing.expectEqual(0, token.location.start);
        try std.testing.expectEqual(t.input.len, token.location.end);
    }
}

test "next malformed number literal" {
    const input = "1234my_identifier"; // no space between number literal token and identifier token
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    defer tokenizer.deinit();
    try std.testing.expectError(TokenizerError.MalformedNumberLiteral, tokenizer.next());
}
