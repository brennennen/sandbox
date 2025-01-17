const std = @import("std");

const Token = @import("./tokenizer.zig").Token;
const Tokenizer = @import("./tokenizer.zig").Tokenizer;
const ast = @import("./ast.zig");
const Parser = @import("./parser.zig").Parser;
const ParserError = @import("./parser.zig").ParserError;
const Environment = @import("./environment.zig").Environment;
const object = @import("./object.zig");
const Object = @import("./object.zig").Object;

pub const TRUE = object.Boolean{ .value = true };
pub const FALSE = object.Boolean{ .value = true };
pub const NIL = object.Nil{};

pub var NIL_OBJ = Object{ .nil = NIL };
pub var TRUE_OBJ = Object{ .boolean = TRUE };
pub var FALSE_OBJ = Object{ .boolean = FALSE };

pub const EvaluatorError = ParserError || error{
    InvalidProgram,
};

pub const Evaluator = struct {
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) Evaluator {
        return .{ .allocator = allocator };
    }

    fn evaluateProgram(
        self: *Evaluator,
        program: ast.Program,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateProgram\n", .{});
        var result: *Object = &NIL_OBJ;
        //var result = object.Object{ .nil = NIL };
        for (program.entry.statements) |statement| {
            result = try self.evaluateStatement(&statement, env);
            switch (result) {
                else => {
                    // TODO
                },
            }
        }
        return result;
    }

    //
    // MARK: Statements
    //
    fn evaluateStatement(
        self: *Evaluator,
        statement: *const ast.Statement,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateStatement\n", .{});

        switch (statement.*) {
            .local => |local| {
                std.debug.print("TODO eval local statement\n", .{});
                _ = local;
            },
            ._return => |_return| {
                return try self.evaluateReturn(&_return, env);
                //std.debug.print("TODO eval _return statement\n", .{});
                //_ = _return;
            },
            .chunk => |chunk| {
                std.debug.print("TODO eval chunk statement\n", .{});
                _ = chunk;
            },
            .expressionStatement => |expressionStatement| {
                std.debug.print("TODO eval expressionStatement statement\n", .{});
                _ = expressionStatement;
            },
        }
        return EvaluatorError.InvalidProgram;
    }

    fn evaluateChunk(
        self: *Evaluator,
        chunk: *const ast.Chunk,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateChunk\n", .{});
        _ = self;
        _ = chunk;
        _ = env;
        return EvaluatorError.InvalidProgram;
    }

    fn evaluateReturn(
        self: *Evaluator,
        returnStatement: *const ast.Return,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateReturn\n", .{});

        const value = try self.evaluateExpression(returnStatement.value, env);
        return value;
    }

    fn nativeBoolToBooleanObject(boolean: bool) *Object {
        if (boolean) {
            return &TRUE_OBJ;
        } else {
            return &FALSE_OBJ;
        }
    }

    //
    // MARK: Expressions
    //

    fn evaluateExpression(
        self: *Evaluator,
        expression: *const ast.Expression,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateExpression\n", .{});
        switch (expression.*) {
            .boolean => |boolean| {
                return nativeBoolToBooleanObject(boolean.value);
            },
            .integer => |integer| {
                return try Object.newInteger(self.allocator, integer.value);
            },
            .infixExpression => |*infixExpression| {
                return try self.evaluateInfixExpression(infixExpression, env);
            },
            else => {
                std.debug.print("TODO evaluateExpression else \n", .{});
            },
        }
        return EvaluatorError.InvalidProgram;
    }

    fn evaluateInfixExpression(
        self: *Evaluator,
        infixExpression: *const ast.InfixExpression,
        env: *Environment,
    ) EvaluatorError!*Object {
        const left = try self.evaluateExpression(infixExpression.left, env);
        const right = try self.evaluateExpression(infixExpression.right, env);
        switch (left.*) {
            .integer => |leftInteger| {
                switch (right.*) {
                    .integer => |rightInteger| {
                        return try self.evaluateIntegerInfixExpression(leftInteger, infixExpression.operator, rightInteger);
                    },
                    else => {
                        std.debug.print("TODO infix type error!\n", .{});
                    },
                }
            },
            else => {
                std.debug.print("TODO infix types\n", .{});
            },
        }
        return EvaluatorError.InvalidProgram;
    }

    fn evaluateIntegerInfixExpression(
        self: *Evaluator,
        left: object.Integer,
        operator: ast.Operator,
        right: object.Integer,
    ) EvaluatorError!*Object {
        switch (operator) {
            .add => {
                return try Object.newInteger(self.allocator, left.value + right.value);
            },
            else => {
                std.debug.print("TODO evaluate_integer_infix_expression\n", .{});
            },
        }
        return EvaluatorError.InvalidProgram;
    }
};

//
// MARK: Tests
//

const EvaluatorTestError = EvaluatorError || error{
    TypeMismatch,
};

pub fn testEval(input: [:0]const u8, allocator: std.mem.Allocator) EvaluatorTestError!*object.Object {
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    var parser = try Parser.init(&tokenizer, allocator);
    const program = try parser.parseProgram();
    var environment = Environment.init(allocator);
    var evaluator = Evaluator.init(allocator);
    return try evaluator.evaluateProgram(program, &environment);
}

test "evaluate return bool statement" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: bool,
    };

    const tests = [_]TestCase{
        .{
            .input = "return true",
            .expected = true,
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();
        const actual = try testEval(testCase.input, allocator.allocator());
        try std.testing.expectEqual(testCase.expected, actual.boolean.value);
    }
}

test "evaluate return int statement" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: i64,
    };

    const tests = [_]TestCase{
        .{
            .input = "return 1",
            .expected = 1,
        },
        .{
            .input = "return 1 + 1",
            .expected = 2,
        },
        .{
            .input = "return 1 + 2 + 3",
            .expected = 6,
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();
        const actual = try testEval(testCase.input, allocator.allocator());
        try std.testing.expectEqual(testCase.expected, actual.integer.value);
    }
}
