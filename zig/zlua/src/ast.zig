//! lua "AST" (Abstract Syntax Tree) implementation.
//!
//! Notes:
//! * Polymorphism/interfaces - union enums with a switch case.
//!
//!

const std = @import("std");
//const Token = @import("./Token.zig");
const Token = @import("./tokenizer.zig").Token;

pub const ParserError = error{InvalidProgram};

pub const Operator = enum {
    assign,
    // Arithmetic
    add,
    subtract,
    multiply,
    divide,
    exponent,
    // Bitwise
    // bitwise_and,
    // bitwise_or,
    // bitwise_xor,
    // right_shift,
    // left_shift,
    // bitwise_not,
    // Equality
    negate,
    equal,
    not_equal,
    less_than,
    greater_than,
    less_than_or_equal,
    greater_than_or_equal,
    // and,

    pub fn lexeme(operator: Operator) []const u8 {
        return switch (operator) {
            .assign => "=",
            // Arithmetic
            .add => "+",
            .subtract => "-",
            .multiply => "*",
            .divide => "/",
            .exponent => "^",
            // Bitwise
            // .bitwise_and => "&",
            // .bitwise_or => "|",
            // .bitwise_xor => "~",
            // .right_shift => ">>",
            // .left_shift => "<<",
            // .bitwise_not => "~",
            // Equality
            .negate => "!",
            .equal => "==",
            .not_equal => "!=",
            .less_than => "<",
            .greater_than => ">",
            .less_than_or_equal => "<=",
            .greater_than_or_equal => ">=",
            // Logical
        };
    }

    pub fn fromTag(tag: Token.Tag) ParserError!Operator {
        return switch (tag) {
            Token.Tag.equal => .assign,
            // Arithmetic
            Token.Tag.plus => .add,
            Token.Tag.dash => .subtract,
            Token.Tag.asterisk => .multiply,
            Token.Tag.slash => .divide,
            Token.Tag.caret => .exponent,
            // bitwise
            // Token.Tag.?? => .bitwise_and,
            // Equality
            Token.Tag.bang => .negate,
            Token.Tag.equal_equal => .equal,
            Token.Tag.not_equal => .not_equal,
            Token.Tag.angle_bracket_left => .less_than,
            Token.Tag.angle_bracket_right => .greater_than,
            Token.Tag.angle_bracket_left_equal => .less_than_or_equal,
            Token.Tag.angle_bracket_right_equal => .greater_than_or_equal,
            else => ParserError.InvalidProgram,
        };
    }

    pub fn isOperator(tag: Token.Tag) bool {
        return switch (tag) {
            .plus, .dash, .asterisk, .slash, .caret => true,
            else => false,
        };
    }

    pub fn format(self: Operator, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{lexeme(self)});
    }
};

// MARK: AST Node
pub const Node = union(enum) {
    program: *Program,
    statement: *Statement,
    //expression: *Expression,

    pub fn format(self: Node, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("Program: (statements: {d})\n", .{self.statements.items.len});
        for (self.statements.items) |statement| {
            try writer.print("   {}\n", .{statement});
        }
    }
};

// MARK: AST Root Node
pub const Program = struct {
    statements: std.ArrayList(Statement),

    pub fn format(self: Program, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("Program: (statements: {d})\n", .{self.statements.items.len});
        for (self.statements.items, 0..) |statement, i| {
            try writer.print("[{d}]   {}\n", .{ i, statement });
        }
    }

    // De-initializes any memory allocated by the Program.
    pub fn deinit(self: *Program) void {
        // TODO: deinit any statements that allocated.
        self.statements.deinit();
    }
};

// MARK: Statements
pub const Statement = union(enum) {
    local: Local,
    _return: Return,
    expression: Expression,
    block: Block,

    pub fn format(self: Statement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        return switch (self) {
            .local => try writer.print("{}", .{self.local}),
            ._return => try writer.print("{}", .{self._return}),
            .expression => try writer.print("{}", .{self.expression}),
            .block => try writer.print("{}", .{self.block}),
        };
    }
};

pub const Local = struct {
    name: Identifier,
    value: *Expression,

    pub fn format(self: Local, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("local {} = {}", .{ self.name, self.value });
    }
};

pub const Return = struct {
    value: *Expression,

    pub fn format(self: Return, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("return {}", .{self.value});
    }
};

pub const ExpressionStatement = struct {
    expression: *Expression,

    pub fn format(self: Local, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.expression});
    }
};

pub const Block = struct {
    statements: std.ArrayList(Statement),

    pub fn format(self: Block, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        for (self.statements.items, 0..) |statement, i| {
            try writer.print("[{d}] {}\n", .{ i, statement });
        }
        //try writer.print("", .{});
    }
};

// MARK: Expressions
pub const Expression = union(enum) {
    identifier: Identifier,
    prefixExpression: PrefixExpression,
    infixExpression: InfixExpression,
    integer: Integer,
    boolean: Boolean,
    stringLiteral: StringLiteral,
    ifExpression: IfExpression,

    pub fn format(self: Expression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        return switch (self) {
            .identifier => try writer.print("{}", .{self.identifier}),
            .prefixExpression => try writer.print("{}", .{self.prefixExpression}),
            .infixExpression => try writer.print("{}", .{self.infixExpression}),
            .integer => try writer.print("{}", .{self.integer}),
            .boolean => try writer.print("{}", .{self.boolean}),
            .stringLiteral => try writer.print("{}", .{self.stringLiteral}),
            .ifExpression => try writer.print("{}", .{self.ifExpression}),
        };
    }
};

pub const PrefixExpression = struct {
    operator: Operator,
    right: *Expression,

    pub fn format(self: PrefixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{} {}", .{ self.operator, self.right });
    }
};

pub const InfixExpression = struct {
    left: *Expression,
    operator: Operator,
    right: *Expression,

    pub fn format(self: InfixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("({} {} {})", .{ self.left, self.operator, self.right });
    }
};

pub const IfExpression = struct {
    condition: *Expression,
    consequence: *Block,
    //elseIfs: std.ArrayList(IfExpression),
    alternative: ?*Block,

    pub fn format(self: IfExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("if {} then {}", .{ self.condition, self.consequence });
        // TODO: else ifs?
        if (self.alternative != null) {
            try writer.print(" else {}", .{self.alternative.?});
        }
        try writer.print(" end", .{});
    }
};

// MARK: Misc
pub const Identifier = struct {
    value: []const u8,

    // fn node(self: *Identifier) Node {
    //     return Node.init(self);
    // }

    pub fn format(self: Identifier, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{self.value});
    }
};

pub const Integer = struct {
    value: i64,

    pub fn format(self: Integer, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const Boolean = struct {
    value: bool,

    pub fn format(self: Boolean, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const StringLiteral = struct {
    value: []const u8,

    pub fn format(self: StringLiteral, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{self.value});
    }
};

// MARK: Tests
