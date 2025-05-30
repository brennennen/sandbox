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
    divide_floor,
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
            .divide_floor => "//",
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
            Token.Tag.slash_slash => .divide_floor,
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
    //program: *Program,
    statement: *const Statement,
    boolean: Boolean,
    //expression: *Expression,
    //statement: *const Statement,
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
    entry: Chunk,

    pub fn format(self: Program, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("Program: \n", .{});
        try writer.print("{}\n", .{self.entry});

        // for (self.chunk.statements.items, 0..) |statement, i| {
        //     try writer.print("[{d}]   {}\n", .{ i, statement });
        // }
    }

    // De-initializes any memory allocated by the Program.
    pub fn deinit(self: *Program) void {
        for (self.chunk.statements.items) |*statement| {
            statement.*.deinit();
        }
        self.statements.deinit();
    }
};

// MARK: Statements
pub const Statement = union(enum) {
    local: Local,
    _return: Return,
    expressionStatement: ExpressionStatement,
    chunk: Chunk,

    pub fn format(self: Statement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        return switch (self) {
            inline else => |*case| try writer.print("{}", .{case.*}),
        };
    }

    pub fn deinit(self: *Statement) void {
        switch (self.*) {
            inline else => |*case| case.*.deinit(),
        }
    }
};

pub const Local = struct {
    name: Identifier,
    value: *Expression,

    pub fn format(self: Local, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("local {} = {}", .{ self.name, self.value });
    }

    pub fn deinit(self: *Local) void {
        self.value.deinit();
    }
};

pub const Return = struct {
    value: *Expression,

    pub fn format(self: Return, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("return {}", .{self.value});
    }

    pub fn deinit(self: *Return) void {
        self.value.deinit();
    }
};

// TODO: this was added to make testing the evaluator early on easier.
// lua doesn't actually allow expression statements, so remove all this and make sure an error
// is raised instead down the line.
pub const ExpressionStatement = struct {
    expression: *Expression,

    pub fn format(self: ExpressionStatement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.expression});
    }

    pub fn deinit(self: *ExpressionStatement) void {
        self.expression.deinit();
    }
};

pub const Chunk = struct {
    statements: []const Statement,

    pub fn format(self: Chunk, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        for (self.statements, 0..) |statement, i| {
            try writer.print("[{d}] {}\n", .{ i, statement });
        }
        //try writer.print("", .{});
    }

    pub fn deinit(self: *Chunk) void {
        for (self.statements) |*statement| {
            statement.*.deinit();
        }
        self.statements.deinit();
    }
};

// MARK: Expressions
pub const Expression = union(enum) {
    identifier: Identifier,
    prefixExpression: PrefixExpression,
    infixExpression: InfixExpression,
    integer: Integer,
    boolean: Boolean,
    string: String,
    ifExpression: IfExpression,
    function: Function,

    pub fn format(self: Expression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        return switch (self) {
            inline else => |*case| try writer.print("{}", .{case.*}),
        };
    }

    pub fn deinit(self: *Expression) void {
        switch (self.*) {
            .identifier, .integer, .boolean, .string => {},
            inline else => |*case| case.*.deinit(),
        }
    }
};

pub const PrefixExpression = struct {
    operator: Operator,
    right: *Expression,

    pub fn format(self: PrefixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{} {}", .{ self.operator, self.right });
    }

    pub fn deinit(self: *PrefixExpression) void {
        self.right.deinit();
    }
};

pub const InfixExpression = struct {
    left: *Expression,
    operator: Operator,
    right: *Expression,

    pub fn format(self: InfixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("({} {} {})", .{ self.left, self.operator, self.right });
    }

    pub fn deinit(self: *InfixExpression) void {
        self.left.deinit();
        self.right.deinit();
    }
};

pub const IfExpression = struct {
    condition: *Expression,
    consequence: *Chunk,
    //elseIfs: std.ArrayList(IfExpression),
    // TODO: should elseIfs just be if expressions in the alternative? should alternative be a statement?
    alternative: ?*Chunk,

    pub fn format(self: IfExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("if {} then {}", .{ self.condition, self.consequence });
        // TODO: else ifs?
        if (self.alternative != null) {
            try writer.print(" else {}", .{self.alternative.?});
        }
        try writer.print(" end", .{});
    }

    pub fn deinit(self: *IfExpression) void {
        self.condition.deinit();
        self.consequence.deinit();
        // TODO: else ifs
        if (self.alternative != null) {
            self.alternative.?.deinit();
        }
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

pub const String = struct {
    value: []const u8,

    pub fn format(self: String, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{self.value});
    }
};

pub const Function = struct {
    name: Identifier,
    body: *Chunk,
    //arguments: std.ArrayList(Identifier),
    arguments: []Identifier,

    // pub fn format(self: FunctionLiteral, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
    //     try writer.print("{s}", .{self.value});
    // }
};

pub const CallExpression = struct {};

// MARK: Tests
