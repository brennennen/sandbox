const std = @import("std");

const Token = @import("./tokenizer.zig").Token;
const Tokenizer = @import("./tokenizer.zig").Tokenizer;
const ast = @import("./ast.zig");
const Parser = @import("./parser.zig").Parser;
const ParserError = @import("./parser.zig").ParserError;
const Environment = @import("./environment.zig").Environment;
const EnvironmentError = @import("./environment.zig").EnvironmentError;
const object = @import("./object.zig");
const Object = @import("./object.zig").Object;

pub const TRUE = object.Boolean{ .value = true };
pub const FALSE = object.Boolean{ .value = true };
pub const NIL = object.Nil{};

pub var NIL_OBJ = Object{ .nil = NIL };
pub var TRUE_OBJ = Object{ .boolean = TRUE };
pub var FALSE_OBJ = Object{ .boolean = FALSE };

pub const EvaluatorError = ParserError || EnvironmentError || error{
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
        return try self.evaluateChunk(program.entry, env);
    }

    //
    // MARK: Statements
    //
    fn evaluateStatement(
        self: *Evaluator,
        statement: ast.Statement,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateStatement\n", .{});

        switch (statement) {
            .local => |local| {
                return try self.evaluateLocal(local, env);
            },
            ._return => |_return| {
                return try self.evaluateReturn(_return, env);
            },
            .chunk => |chunk| {
                return try self.evaluateChunk(chunk, env);
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
        chunk: ast.Chunk,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateChunk\n", .{});
        var result: *Object = &NIL_OBJ;
        for (chunk.statements) |statement| {
            result = try self.evaluateStatement(statement, env);
            // TODO: check for return, if return, exit early? or throw error if return isn't last statement? match what c++ lua does
        }
        return result;
    }

    fn evaluateLocal(
        self: *Evaluator,
        local: ast.Local,
        env: *Environment,
    ) EvaluatorError!*Object {
        std.debug.print("evaluateLocal\n", .{});
        const value = try self.evaluateExpression(local.value, env);
        try env.*.set(local.name.value, value);
        return &NIL_OBJ;
    }

    fn evaluateReturn(
        self: *Evaluator,
        returnStatement: ast.Return,
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
            .subtract => {
                return try Object.newInteger(self.allocator, left.value - right.value);
            },
            .multiply => {
                return try Object.newInteger(self.allocator, left.value * right.value);
            },
            // .divide => {
            //     // TODO return float? need to think about how to handle numbers better...
            //     return try Object.newInteger(self.allocator, left.value / right.value);
            // },
            .divide_floor => {
                const result = @divFloor(left.value, right.value);
                return try Object.newInteger(self.allocator, result);
            },
            .exponent => {
                const result = std.math.pow(i64, left.value, right.value);
                return try Object.newInteger(self.allocator, result);
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
    TestExpectedEqual,
    UnexpectedStatement,
    UnexpectedExpression,
};

pub fn testEval(input: [:0]const u8, allocator: std.mem.Allocator) EvaluatorTestError!*object.Object {
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    var parser = try Parser.init(&tokenizer, allocator);
    const program = try parser.parseProgram();
    var environment = Environment.init(allocator);
    var evaluator = Evaluator.init(allocator);
    return try evaluator.evaluateProgram(program, &environment);
}

pub fn testEvalEnv(input: [:0]const u8, env: *Environment, allocator: std.mem.Allocator) EvaluatorTestError!*object.Object {
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    var parser = try Parser.init(&tokenizer, allocator);
    const program = try parser.parseProgram();
    var evaluator = Evaluator.init(allocator);
    return try evaluator.evaluateProgram(program, env);
}

pub fn expectObject(expected: Object, actual: Object) EvaluatorTestError!void {
    switch (expected) {
        .integer => |expectedInteger| {
            switch (actual) {
                .integer => |actualInteger| {
                    //try expectInteger(expectedInteger, actualInteger);
                    try std.testing.expectEqual(expectedInteger.value, actualInteger.value);
                },
                else => {
                    return EvaluatorTestError.TypeMismatch;
                },
            }
        },
        .boolean => |expectedBoolean| {
            _ = expectedBoolean;
        },
        .nil => |expectedNil| {
            _ = expectedNil;
        },
        else => {
            // TODO!
            return EvaluatorTestError.InvalidProgram;
        },
    }
}

pub fn expectInteger(expected: object.Integer, actual: object.Integer) EvaluatorTestError!void {
    try std.testing.expectEqual(expected.value, actual.value);
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

test "evaluate local statements" {
    const LocalResult = struct {
        name: []const u8,
        value: Object,
    };
    const TestCase = struct {
        input: [:0]const u8,
        expectedLocals: []const LocalResult,
    };

    const tests = [_]TestCase{
        .{
            .input = "local x = 1",
            .expectedLocals = &.{
                LocalResult{
                    .name = "x",
                    .value = Object{ .integer = object.Integer{ .value = 1 } },
                },
            },
        },
        .{
            .input = "local foo = 1 + 2 * 3",
            .expectedLocals = &.{
                LocalResult{
                    .name = "foo",
                    .value = Object{ .integer = object.Integer{ .value = 7 } },
                },
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var env = Environment.init(allocator.allocator());
        const actual = try testEvalEnv(testCase.input, &env, allocator.allocator());
        try std.testing.expectEqual(&NIL_OBJ, actual);
        for (testCase.expectedLocals) |expectedLocal| {
            const localValue = env.get(expectedLocal.name);
            try std.testing.expect(localValue != null);
            try expectObject(expectedLocal.value, localValue.?.*);
        }
    }
}
