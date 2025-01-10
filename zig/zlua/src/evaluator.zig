const std = @import("std");

const Token = @import("./tokenizer.zig").Token;
const Tokenizer = @import("./tokenizer.zig").Tokenizer;
const ast = @import("./ast.zig");
const Parser = @import("./parser.zig").Parser;
const Environment = @import("./environment.zig").Environment;
const object = @import("./object.zig");

pub const TRUE = object.Boolean{ .value = true };
pub const FALSE = object.Boolean{ .value = true };
pub const NIL = object.Nil{};

pub const Evaluator = struct {
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) Evaluator {
        return .{ .allocator = allocator };
    }

    fn evaluate(self: *Evaluator, node: ast.Node, env: *Environment) object.Object {
        switch (node) {
            .program => {
                return self.evaluateProgram(node, env);
            },
            .statement => {},
            // .expression => {},
        }

        return object.Object{ .nil = NIL };
    }

    fn evaluateProgram(self: *Evaluator, node: ast.Node, env: *Environment) object.Object {
        std.debug.print("evaluateProgram\n", .{});
        var result = object.Object{ .nil = NIL };
        for (node.program.statements.items) |statement| {
            result = self.evaluate(ast.Node{ .statement = &statement }, env);
            switch (result) {}
        }

        return object.Object{ .nil = NIL };
    }
};

//
// MARK: Tests
//

pub fn testEval(input: [:0]const u8, allocator: std.mem.Allocator) !object.Object {
    var tokenizer = Tokenizer.init(std.testing.allocator, input);
    var parser = Parser.init(&tokenizer, allocator);
    var program = try parser.parseProgram();
    const node = ast.Node{ .program = &program };
    var environment = Environment.init(allocator);
    var evaluator = Evaluator.init(allocator);
    return evaluator.evaluate(node, &environment);
}

test "evaluate boolean expression" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: bool,
    };

    const tests = [_]TestCase{
        .{
            .input = "true",
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
