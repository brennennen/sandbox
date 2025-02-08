//!
//! Tokenizer -> *Parser* -> Evaluater
//!
//! Takes tokenized input and parses it to create the AST (Abstract Syntax Tree).
//!
//! Notes:
//! * Performs "Pratt Parsing" (a form of recursive descent with some optimizations
//! to make operator precedence management cleaner).
//!

const std = @import("std");
const Token = @import("./tokenizer.zig").Token;
const Tokenizer = @import("./tokenizer.zig").Tokenizer;
const TokenizerError = @import("./tokenizer.zig").TokenizerError;
const ast = @import("./ast.zig");

pub const ParserError = TokenizerError || error{
    InvalidProgram,
    UnexpectedToken,
    OutOfMemory,
};

///
///  TODO: some operators are "left associative" and some are "right associative"
/// https://www.lua.org/pil/3.5.html
///
const Precedence = enum(u8) {
    lowest = 0,
    _or = 1,
    _and = 2,
    equals = 3,
    lessgreater = 4,
    sum = 5,
    product = 6,
    prefix = 7,
    call = 8,
    index = 9,
    exponentiation = 10, // highest precedence

    fn fromTag(tag: Token.Tag) Precedence {
        switch (tag) {
            // Equality
            Token.Tag.equal_equal => return Precedence.equals,
            Token.Tag.not_equal => return Precedence.equals,
            Token.Tag.angle_bracket_left => return Precedence.lessgreater,
            Token.Tag.angle_bracket_right => return Precedence.lessgreater,
            Token.Tag.angle_bracket_left_equal => return Precedence.lessgreater,
            Token.Tag.angle_bracket_right_equal => return Precedence.lessgreater,
            // Arithmetic
            Token.Tag.plus => return Precedence.sum,
            Token.Tag.dash => return Precedence.sum,
            Token.Tag.asterisk => return Precedence.product,
            Token.Tag.slash => return Precedence.product,
            Token.Tag.slash_slash => return Precedence.product,
            Token.Tag.caret => return Precedence.exponentiation,
            else => return .lowest,
        }
    }

    fn fromOperator(operator: ast.Operator) Precedence {
        switch (operator) {
            // Equality
            ast.Operator.equal => return Precedence.equals,
            ast.Operator.not_equal => return Precedence.equals,
            ast.Operator.less_than => return Precedence.lessgreater,
            ast.Operator.greater_than => return Precedence.lessgreater,
            ast.Operator.less_than_or_equal => return Precedence.lessgreater,
            ast.Operator.greater_than_or_equal => return Precedence.lessgreater,
            // Arithmetic
            ast.Operator.add => return Precedence.sum,
            ast.Operator.subtract => return Precedence.sum,
            ast.Operator.multiply => return Precedence.product,
            ast.Operator.divide => return Precedence.product,
            ast.Operator.divide_floor => return Precedence.product,
            ast.Operator.exponent => return Precedence.exponentiation,
            else => return .lowest,
        }
    }

    pub fn format(self: Precedence, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{std.enums.tagName(Precedence, self)});
    }
};

pub const Parser = struct {
    tokenizer: *Tokenizer,
    errors: std.ArrayList([]const u8),
    token: Token,
    next_token: Token,
    allocator: std.mem.Allocator,
    // TODO: keep track of current expression depth and exit out if too deep?

    pub fn init(tokenizer: *Tokenizer, allocator: std.mem.Allocator) ParserError!Parser {
        //var errors = std.ArrayList([]const u8).init(allocator);
        const tok = try tokenizer.next();
        const next_token = try tokenizer.next();
        const parser = Parser{
            .allocator = allocator,
            .tokenizer = tokenizer,
            .errors = std.ArrayList([]const u8).init(allocator),
            .token = tok,
            .next_token = next_token,
        };
        return parser;
    }

    // De-initializes any memory allocated by the parser.
    pub fn deinit(self: *Parser) void {
        self.errors.deinit();
        //self.new_lines.deinit();
    }

    /// Retrieves the next token from the source file.
    pub fn next(self: *Parser) TokenizerError!void {
        self.token = self.next_token;
        self.next_token = try self.tokenizer.next();
    }

    //pub fn error(self: *Parser, tag: Diagnostics.Tag)

    pub fn parseProgram(self: *Parser) ParserError!ast.Program {
        std.debug.print("parseProgram: '{s}'\n", .{self.tokenizer.buffer});
        var statements = std.ArrayList(ast.Statement).init(self.allocator);
        var sentinal: i32 = 0;
        while (self.token.tag != .eof) {
            sentinal += 1;
            if (sentinal >= 10) {
                std.debug.print("ERROR! hit sentinal max statements in program.\n", .{});
                break;
            }
            std.debug.print("   parsing stmt: {d}\n", .{statements.items.len});
            const statement = try self.parseStatement();
            try statements.append(statement);
        }

        return ast.Program{ .entry = ast.Chunk{ .statements = try statements.toOwnedSlice() } };
    }

    //
    // MARK: Primitive/Identifier Parsing
    //
    // Parses the current token as an indentifier and moves the token pointer forward if successful.
    fn parseIdentifier(self: *Parser) ParserError!ast.Identifier {
        switch (self.token.tag) {
            .identifier => {
                const identifier = ast.Identifier{
                    .value = self.token.getValue(self.tokenizer.buffer),
                };
                return identifier;
            },
            else => return ParserError.UnexpectedToken,
        }
    }

    // Parses the current token as an integer and moves the token pointer forward if successful.
    fn parseInteger(self: *Parser) ParserError!ast.Integer {
        switch (self.token.tag) {
            .number_literal => {
                const token = self.token.getValue(self.tokenizer.buffer);
                // TODO: check for hex string literals "0x" and handle accordingly
                if (std.fmt.parseInt(i64, token, 10)) |number| {
                    //try self.next();
                    return ast.Integer{ .value = number };
                } else |err| {
                    std.debug.print("parseInteger error: {s}\n", .{@errorName(err)});
                    return ParserError.InvalidProgram;
                }
            },
            else => return ParserError.UnexpectedToken,
        }
    }

    fn parseBoolean(self: *Parser) ParserError!ast.Boolean {
        switch (self.token.tag) {
            .keyword_true => {
                return ast.Boolean{ .value = true };
            },
            .keyword_false => {
                return ast.Boolean{ .value = false };
            },
            else => return ParserError.UnexpectedToken,
        }
    }

    fn parseString(self: *Parser) ParserError!ast.String {
        if (self.token.tag == .string_literal) {
            const stringToken = self.token.getValue(self.tokenizer.buffer);
            // Strip off the quotation marks.
            return ast.String{ .value = stringToken[1 .. stringToken.len - 1] };
        } else {
            return ParserError.UnexpectedToken;
        }
    }

    // TODO: parse grouped expression? Chunk expression?
    ///
    ///
    ///
    fn parseGroupedExpression(self: *Parser) ParserError!ast.Expression {
        try self.next();
        const expression = try self.parseExpression(.lowest);
        if (self.next_token.tag != .parentheses_right) {
            return ParserError.UnexpectedToken;
        }
        try self.next();
        return expression;
    }

    /// if <expression> then <statements> end
    /// if <expression> then <statements> else <statements> end
    /// if <expression> then <statements> elseif <expression> then <statements> else <statements> end
    fn parseIfExpression(self: *Parser) ParserError!ast.IfExpression {
        std.debug.print("parseIfExpression\n", .{});
        try self.next();

        const condition = try self.allocator.create(ast.Expression);
        condition.* = try self.parseExpression(Precedence.lowest);
        try self.next();

        if (self.token.tag == .keyword_then) {
            try self.next();
        } else {
            return ParserError.InvalidProgram; // TODO: throw better error
        }

        const consequence = try self.allocator.create(ast.Chunk);
        consequence.* = try self.parseChunkStatement();
        var ifExpression = ast.IfExpression{
            .condition = condition,
            .consequence = consequence,
            .alternative = null,
        };

        if (self.token.tag == .keyword_end) {
            try self.next();
            return ifExpression;
        }

        // TODO: implement elseif as just nested if elses?

        if (self.token.tag == .keyword_else) {
            try self.next();
            const alternative = try self.allocator.create(ast.Chunk);
            alternative.* = try self.parseChunkStatement();
            ifExpression.alternative = alternative;
        }

        // end
        if (self.token.tag == .keyword_end) {
            std.debug.print("ifExpression if then else end\n", .{});
            try self.next();
            return ifExpression;
        }

        std.debug.print("malformed ifExpression. last tok: {}, expr: {}\n", .{ self.token.tag, ifExpression });
        return ParserError.InvalidProgram; // TODO: throw better error
    }

    /// function <name>(<args>) <chunk> end
    /// function foo() return 5 end
    pub fn parseFunction(self: *Parser) ParserError!ast.Function {
        std.debug.print("parseFunction: tok: {} {s}\n", .{ self.token.tag, self.token.getValue(self.tokenizer.buffer) });

        // function
        try self.next();
        // <name>
        const ident = try parseIdentifier(self);
        try self.next();

        if (self.token.tag != .parentheses_left) {
            std.debug.print("function missing left paren\n", .{});
            return ParserError.InvalidProgram;
        }

        // (<args>)
        const arguments = try parseFunctionArguments(self);

        // <chunk>
        const chunk = try self.allocator.create(ast.Chunk);
        chunk.* = try parseChunkStatement(self);

        // end
        if (self.token.tag == .keyword_end) {
            try self.next();
            return ast.Function{ .name = ident, .arguments = arguments, .body = chunk };
        }

        return ParserError.InvalidProgram;
    }

    pub fn parseFunctionArguments(self: *Parser) ParserError![]ast.Identifier {
        if (self.token.tag != .parentheses_left) {
            std.debug.print("function missing left paren\n", .{});
            return ParserError.InvalidProgram;
        }
        try self.next();

        var arguments = std.ArrayList(ast.Identifier).init(self.allocator);
        // Return an empty list if there are no arguments.
        if (self.token.tag == .parentheses_right) {
            try self.next();
            return try arguments.toOwnedSlice();
        }

        //std.debug.print("first arg: tok: {} {s}\n", .{ self.token.tag, self.token.getValue(self.tokenizer.buffer) });
        try arguments.append(try self.parseIdentifier());
        try self.next();

        while (self.token.tag == Token.Tag.comma) {
            try self.next();
            try arguments.append(try self.parseIdentifier());
            try self.next();
        }

        if (self.token.tag == .parentheses_right) {
            try self.next();
            //std.debug.print("done parsing function arguments: tok: {} {s}\n", .{ self.token.tag, self.token.getValue(self.tokenizer.buffer) });
            return try arguments.toOwnedSlice();
        }

        return ParserError.InvalidProgram;
    }

    //
    // MARK: Statement Parsing
    //
    pub fn parseStatement(self: *Parser) ParserError!ast.Statement {
        std.debug.print("parseStatement: tok: {} {s}\n", .{ self.token.tag, self.token.getValue(self.tokenizer.buffer) });
        switch (self.token.tag) {
            .keyword_local => {
                return ast.Statement{ .local = try self.parseLocalStatement() };
            },
            .keyword_return => {
                return ast.Statement{ ._return = try self.parseReturnStatement() };
            },
            else => {
                return ast.Statement{ .expressionStatement = try self.parseExpressionStatement() };
            },
        }
        std.debug.print("parseStatement: Failed! token: {?s} not supported yet.\n", .{self.token.getValue(self.tokenizer.buffer)});
        return ParserError.InvalidProgram;
    }

    /// local <identifier> = <expression>
    pub fn parseLocalStatement(self: *Parser) ParserError!ast.Local {
        std.debug.print("parseLocalStatement. tok: {s}\n", .{
            self.token.getValue(self.tokenizer.buffer),
        });
        if (self.expectNextToken(Token.Tag.identifier) != true) {
            return ParserError.InvalidProgram;
        }

        try self.next();
        const identifier = try self.parseIdentifier();
        //const name = identifierFromToken(self.token);
        try self.next();

        if (self.expectToken(Token.Tag.equal) != true) {
            return ParserError.InvalidProgram;
        }
        try self.next();

        const expression = self.allocator.create(ast.Expression) catch return ParserError.InvalidProgram;
        expression.* = try self.parseExpression(.lowest);
        try self.next();

        // LUA has optional semicolons. Eat the semicolon if it's present.
        if (self.token.tag == .semicolon) {
            try self.next();
        }

        //std.debug.print("successfully parsed local statement. ident: '{}'\n", .{identifier});
        //std.debug.print("tok: {s}, next_token: {s}\n", .{ token.Utils.toString(self.token), token.Utils.toString(self.next_token) });
        return ast.Local{ .name = identifier, .value = expression };
        //return ParserError.InvalidProgram;
    }

    pub fn parseReturnStatement(self: *Parser) ParserError!ast.Return {
        try self.next();

        const expression = self.allocator.create(ast.Expression) catch return ParserError.InvalidProgram;
        expression.* = try self.parseExpression(.lowest);
        try self.next();

        if (self.token.tag == .semicolon) {
            try self.next();
        }
        return ast.Return{ .value = expression };
    }

    pub fn parseExpressionStatement(self: *Parser) ParserError!ast.ExpressionStatement {
        const expression = try self.allocator.create(ast.Expression);
        expression.* = try self.parseExpression(.lowest);
        try self.next();
        if (self.token.tag == .semicolon) {
            try self.next();
        }
        return ast.ExpressionStatement{ .expression = expression };
    }

    pub fn isTokenChunkEnd(token: Token) bool {
        if (token.tag == .keyword_end or
            token.tag == .keyword_elseif or
            token.tag == .keyword_else)
        {
            return true;
        } else {
            return false;
        }
    }

    pub fn parseChunkStatement(self: *Parser) ParserError!ast.Chunk {
        std.debug.print("parseChunkStatement: tok: {}\n", .{self.token.tag});
        var _statements = std.ArrayList(ast.Statement).init(self.allocator);

        while (!isTokenChunkEnd(self.token)) {
            try _statements.append(try parseStatement(self));
        }
        return ast.Chunk{ .statements = try _statements.toOwnedSlice() };
    }

    //
    // MARK: MISC
    //

    pub fn expectToken(self: Parser, expectedTokenTag: Token.Tag) bool {
        if (@intFromEnum(self.token.tag) == @intFromEnum(expectedTokenTag)) {
            return true;
        } else {
            return false;
        }
    }

    pub fn expectNextToken(
        self: Parser,
        expectedTokenTag: Token.Tag,
    ) bool {
        if (@intFromEnum(self.next_token.tag) == @intFromEnum(expectedTokenTag)) {
            return true;
        } else {
            return false;
        }
    }

    pub fn identifierFromToken(tok: Token) ast.Identifier {
        const identifier = switch (tok) {
            .identifier => |val| ast.Identifier{ .value = val },
            else => unreachable,
        };
        return identifier;
    }

    //
    // MARK: Expression Parsing
    //

    // parse by prefix token, real prefix expressions are just ! and -
    fn parsePrefixTokenExpression(
        self: *Parser,
        tag: Token.Tag,
    ) ParserError!ast.Expression {
        // std.debug.print("parsePrefixTokenExpression. tok: {s}\n", .{
        //     self.token.getValue(self.tokenizer.buffer),
        // });
        switch (tag) {
            .identifier => {
                return ast.Expression{ .identifier = try self.parseIdentifier() };
            },
            .number_literal => {
                return ast.Expression{ .integer = try self.parseInteger() };
            },
            .dash, .bang => {
                return ast.Expression{ .prefixExpression = try self.parsePrefixOperatorExpression() };
            },
            .keyword_true, .keyword_false => {
                return ast.Expression{ .boolean = try self.parseBoolean() };
            },
            .parentheses_left => {
                return try self.parseGroupedExpression();
            },
            .string_literal => {
                return ast.Expression{ .string = try self.parseString() };
            },
            .keyword_if => {
                return ast.Expression{ .ifExpression = try self.parseIfExpression() };
            },
            .keyword_function => {
                return ast.Expression{ .function = try self.parseFunction() };
            },
            else => {
                std.debug.print("invalid prefix token:{}\n", .{self.token.tag});
                return ParserError.InvalidProgram;
            },
        }
    }

    ///
    ///
    ///
    fn parsePrefixOperatorExpression(self: *Parser) ParserError!ast.PrefixExpression {
        // std.debug.print("parsePrefixOperatorExpression. tok: {s}\n", .{
        //     self.token.getValue(self.tokenizer.buffer),
        // });
        //const operator = ast.Operator.plus;
        const operator = try ast.Operator.fromTag(self.token.tag);
        try self.next();

        const right = try self.parseExpression(.prefix);
        const rightPtr = self.allocator.create(ast.Expression) catch return ParserError.InvalidProgram;
        rightPtr.* = right;

        return ast.PrefixExpression{ .operator = operator, .right = rightPtr };
    }

    ///
    /// Parses the infix expression based on the middle (operator) token.
    ///
    fn parseInfixTokenExpression(
        self: *Parser,
        tok: Token.Tag,
        left: *ast.Expression,
    ) ParserError!ast.Expression {
        // std.debug.print("parseInfixTokenExpression. tok: {}\n", .{tok});
        try self.next();
        switch (tok) {
            // Arithmetic
            .plus,
            .dash,
            .asterisk,
            .slash,
            .slash_slash,
            .caret,
            // Logical
            .equal_equal,
            .not_equal,
            .angle_bracket_left,
            .angle_bracket_right,
            .angle_bracket_left_equal,
            .angle_bracket_right_equal,
            => {
                return ast.Expression{
                    .infixExpression = try self.parseInfixOperatorExpression(left),
                };
            },
            // .parentheses_left => {
            //     // todo
            //     return ast.Expression{
            //         .call = parseCallExpression
            //     };
            // },
            else => {
                return ParserError.InvalidProgram;
            },
        }
    }

    fn parseInfixOperatorExpression(
        self: *Parser,
        left: *ast.Expression,
    ) ParserError!ast.InfixExpression {
        // std.debug.print("parseInfixOperatorExpression. tok: {s}\n", .{
        //     self.token.getValue(self.tokenizer.buffer),
        // });
        const operator = try ast.Operator.fromTag(self.token.tag);
        const precedence = Precedence.fromOperator(operator);

        try self.next();

        const right = try self.parseExpression(precedence);
        const rightPtr = try self.allocator.create(ast.Expression);
        rightPtr.* = right;
        return ast.InfixExpression{
            .left = left,
            .operator = operator,
            .right = rightPtr,
        };
    }

    ///
    /// Top down recursive descent "Pratt" parser.
    /// https://www.lua.org/manual/5.4/manual.html#3.4
    ///
    pub fn parseExpression(
        self: *Parser,
        precedence: Precedence,
    ) ParserError!ast.Expression {
        var leftExpression = try self.parsePrefixTokenExpression(self.token.tag);
        // This while loop is the meat and potatos of the Pratt parsing and creating the AST
        while (self.next_token.tag != .semicolon and
            @intFromEnum(precedence) < @intFromEnum(Precedence.fromTag(self.next_token.tag)))
        {
            // TODO: pop out early if no infix parse function for token
            //std.debug.print("   ast, tok: {}\n", .{self.next_token.tag});
            const leftExpressionPointer = try self.allocator.create(ast.Expression);
            leftExpressionPointer.* = leftExpression;
            leftExpression = try self.parseInfixTokenExpression(
                self.next_token.tag,
                leftExpressionPointer,
            );
        }
        std.debug.print("parseExpression: {} (next tok: {})\n", .{ leftExpression, self.token.tag });
        return leftExpression;
    }

    //
    // MARK: Misc Parsing
    //

};

// All code below this point is only used for testing.

//
// MARK: Test Helpers
//
pub const ParserTestError = ParserError || error{
    TestExpectedEqual,
    UnexpectedStatement,
    UnexpectedExpression,
};

fn expectStatement(expected: *const ast.Statement, actual: *const ast.Statement) ParserTestError!void {
    switch (expected.*) {
        .local => |expectedLocal| {
            switch (actual.*) {
                .local => |actualLocal| {
                    try expectLocal(expectedLocal, actualLocal);
                },
                else => {
                    std.debug.print("expectStatement: unknown actual statement type: {}\n", .{actual});
                    return error.UnexpectedStatement;
                },
            }
        },
        ._return => |expectedReturn| {
            switch (actual.*) {
                ._return => |actualReturn| {
                    try expectReturnStatement(expectedReturn, actualReturn);
                },
                else => {
                    std.debug.print("expectStatement: unknown actual statement type: {}\n", .{actual});
                    return error.UnexpectedStatement;
                },
            }
        },
        .expressionStatement => |expectedExpressionStatement| {
            switch (actual.*) {
                .expressionStatement => |actualExpressionStatement| {
                    try expectExpressionStatement(expectedExpressionStatement, actualExpressionStatement);
                },
                else => {
                    std.debug.print("expectStatement: unknown actual statement type: {}\n", .{actual});
                    return error.UnexpectedStatement;
                },
            }
        },
        .chunk => |expectedChunk| {
            switch (actual.*) {
                .chunk => |actualChunk| {
                    try expectChunk(expectedChunk, actualChunk);
                },
                else => {
                    std.debug.print("\n", .{});
                },
            }
        },
        // else => {
        //     std.debug.print("expectStatement: Bad test, unknown expected statement type: {}\n", .{expected});
        //     return error.UnexpectedStatement;
        // },
    }
}

fn expectLocal(expected: ast.Local, actual: ast.Local) ParserTestError!void {
    try expectIdentifier(expected.name, actual.name);
    try expectExpression(expected.value.*, actual.value.*);
}

fn expectReturnStatement(expected: ast.Return, actual: ast.Return) ParserTestError!void {
    try expectExpression(expected.value.*, actual.value.*);
}

fn expectExpressionStatement(expected: ast.ExpressionStatement, actual: ast.ExpressionStatement) ParserTestError!void {
    try expectExpression(expected.expression.*, actual.expression.*);
}

fn expectIdentifier(expected: ast.Identifier, actual: ast.Identifier) ParserTestError!void {
    try std.testing.expectEqualStrings(expected.value, actual.value);
}

fn expectExpression(expected: ast.Expression, actual: ast.Expression) ParserTestError!void {
    switch (expected) {
        .integer => |expectedInteger| {
            switch (actual) {
                .integer => |actualInteger| {
                    try expectInteger(expectedInteger, actualInteger);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .identifier => |expectedIdentifier| {
            switch (actual) {
                .identifier => |actualIdentifier| {
                    try expectIdentifier(expectedIdentifier, actualIdentifier);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .string => |expectedString| {
            switch (actual) {
                .string => |actualString| {
                    try expectString(expectedString, actualString);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .boolean => |expectedBool| {
            switch (actual) {
                .boolean => |actualBool| {
                    try expectBoolean(expectedBool, actualBool);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .prefixExpression => |expectedPrefixExpression| {
            switch (actual) {
                .prefixExpression => |actualPrefixExpression| {
                    try expectPrefixExpression(expectedPrefixExpression, actualPrefixExpression);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .infixExpression => |expectedInfixExpression| {
            switch (actual) {
                .infixExpression => |actualInfixExpression| {
                    try expectInfixExpression(expectedInfixExpression, actualInfixExpression);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        else => {
            return error.UnexpectedExpression;
        },
    }
}

fn expectChunk(expected: ast.Chunk, actual: ast.Chunk) ParserTestError!void {
    try std.testing.expectEqual(
        expected.statements.len,
        actual.statements.len,
    );

    for (expected.statements, actual.statements) |expStmt, actStmt| {
        try expectStatement(&expStmt, &actStmt);
    }
}

fn expectIfExpression(expected: ast.IfExpression, actual: ast.IfExpression) ParserTestError!void {
    try expectExpression(expected.condition, actual.condition);
    try expectChunk(expected.consequence.*, actual.consequence.*);

    // TODO: expect elseifs!

    if (expected.alternative != null) {
        try expectChunk(expected.alternative.?.*, actual.alternative.?.*);
    }
}

fn expectFunction(expected: ast.Function, actual: ast.Function) ParserTestError!void {
    try expectIdentifier(expected.name, actual.name);
    // TODO: check arguments
    try expectChunk(expected.body.*, actual.body.*);
}

fn expectPrefixExpression(expected: ast.PrefixExpression, actual: ast.PrefixExpression) ParserTestError!void {
    try std.testing.expectEqual(expected.operator, actual.operator);
    try expectExpression(expected.right.*, actual.right.*);
}

fn expectInfixExpression(expected: ast.InfixExpression, actual: ast.InfixExpression) ParserTestError!void {
    try expectExpression(expected.left.*, actual.left.*);
    try std.testing.expectEqual(expected.operator, actual.operator);
    try expectExpression(expected.right.*, actual.right.*);
}

fn expectInteger(expected: ast.Integer, actual: ast.Integer) ParserTestError!void {
    try std.testing.expectEqual(expected.value, actual.value);
}

fn expectString(expected: ast.String, actual: ast.String) ParserTestError!void {
    try std.testing.expectEqualStrings(expected.value, actual.value);
}

fn expectBoolean(expected: ast.Boolean, actual: ast.Boolean) ParserTestError!void {
    try std.testing.expectEqual(expected.value, actual.value);
}

//
// MARK: Tests
//

// Their are 2 main types of tests below: "ast tests" and "string tests".
// "ast tests" are tests that parse a string and assert that the parsed program matches
// the provided ast. It is very inconvient to have to write out ASTs though, so for some
// more adhoc/lazy tests, I "stringify" the ast and check the parsed programs ast against
// this string version instead, these are the "string tests".

// Test the expression pratt parsing and generation of the AST.
test "parse arithmetic infix expression tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.InfixExpression,
    };

    const tests = [_]TestCase{
        .{
            .input = "1 + 2",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 1 },
                }),
                .operator = ast.Operator.add,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 2 },
                }),
            },
        },
        .{
            .input = "34 - 567",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 34 },
                }),
                .operator = ast.Operator.subtract,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 567 },
                }),
            },
        },
        .{
            .input = "89 * 0",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 89 },
                }),
                .operator = ast.Operator.multiply,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 0 },
                }),
            },
        },
        .{
            .input = "12 / 3",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 12 },
                }),
                .operator = ast.Operator.divide,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 3 },
                }),
            },
        },
        .{
            .input = "140 // 20",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 140 },
                }),
                .operator = ast.Operator.divide_floor,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 20 },
                }),
            },
        },
        .{
            .input = "5 + 6 - 7",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 5 },
                        }),
                        .operator = ast.Operator.add,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 6 },
                        }),
                    },
                }),
                .operator = ast.Operator.subtract,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 7 },
                }),
            },
        },
        .{
            .input = "-1 + 2",
            .expected = ast.InfixExpression{
                .left = @constCast(&ast.Expression{
                    .prefixExpression = ast.PrefixExpression{
                        .operator = .subtract,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 1 },
                        }),
                    },
                }),
                .operator = ast.Operator.add,
                .right = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 2 },
                }),
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();

        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        try expectInfixExpression(testCase.expected, program.entry.statements[0].expressionStatement.expression.infixExpression);
    }
}

// Doing the above ad-hoc struct defining of the expression AST is time consuming and error prone.
// The "format" method implemented on each node should allow for a consistent "toString" output that
// can be checked more easily.
test "parse single statement arithmetic expression string tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: [:0]const u8,
    };

    const tests = [_]TestCase{
        .{
            .input = "1 + 2",
            .expected = "(1 + 2)",
        },
        .{
            .input = "1 + 2 + 3 + 4 + 5",
            .expected = "((((1 + 2) + 3) + 4) + 5)",
        },
        .{
            .input = "1 * 2 - 3 / 4 + 5",
            .expected = "(((1 * 2) - (3 / 4)) + 5)",
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();

        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        const actual = try std.fmt.allocPrint(allocator.allocator(), "{}", .{
            program.entry.statements[0],
        });
        try std.testing.expectEqualStrings(testCase.expected, actual);
    }
}

test "parse single statement equality expression string tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: [:0]const u8,
    };

    const tests = [_]TestCase{
        .{
            .input = "1 == 2",
            .expected = "(1 == 2)",
        },
        .{
            .input = "1 != 2",
            .expected = "(1 != 2)",
        },
        .{
            .input = "1 < 2",
            .expected = "(1 < 2)",
        },
        .{
            .input = "1 > 2",
            .expected = "(1 > 2)",
        },
        .{
            .input = "1 >= 2",
            .expected = "(1 >= 2)",
        },
        .{
            .input = "1 <= 2",
            .expected = "(1 <= 2)",
        },
        .{
            .input = "true == false",
            .expected = "(true == false)",
        },
        .{
            .input = "'abcd' == 'efgh'",
            .expected = "(abcd == efgh)",
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();

        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        const actual = try std.fmt.allocPrint(allocator.allocator(), "{}", .{
            program.entry.statements[0],
        });
        try std.testing.expectEqualStrings(testCase.expected, actual);
    }
}

//
// MARK: IfExpression Tests
//
test "parse IfExpression if else tests" {
    var expected_allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer expected_allocator.deinit();

    const TestCase = struct {
        input: [:0]const u8,
        condition: ast.Expression,
        consequence: ast.Statement, // instead of a Chunk, only allowing a single statement to make testing logic simpler.
        alternative: ?ast.Statement, // same as above.
    };

    const tests = [_]TestCase{
        .{
            .input = "if (1 == 2) then return 3 end",
            .condition = ast.Expression{
                .infixExpression = ast.InfixExpression{
                    .left = @constCast(&ast.Expression{
                        .integer = ast.Integer{ .value = 1 },
                    }),
                    .operator = ast.Operator.equal,
                    .right = @constCast(&ast.Expression{
                        .integer = ast.Integer{ .value = 2 },
                    }),
                },
            },
            .consequence = ast.Statement{ ._return = ast.Return{
                .value = @constCast(&ast.Expression{
                    .integer = ast.Integer{ .value = 3 },
                }),
            } },
            .alternative = null,
        },
        .{
            .input = "if (x == true) then return 'alice' else return 'bob' end",
            .condition = ast.Expression{
                .infixExpression = ast.InfixExpression{
                    .left = @constCast(&ast.Expression{
                        .identifier = ast.Identifier{ .value = "x" },
                    }),
                    .operator = ast.Operator.equal,
                    .right = @constCast(&ast.Expression{
                        .boolean = ast.Boolean{ .value = true },
                    }),
                },
            },
            .consequence = ast.Statement{
                ._return = ast.Return{ .value = @constCast(
                    &ast.Expression{ .string = ast.String{
                        .value = "alice",
                    } },
                ) },
            },
            .alternative = ast.Statement{
                ._return = ast.Return{ .value = @constCast(
                    &ast.Expression{ .string = ast.String{
                        .value = "bob",
                    } },
                ) },
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();
        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        // Assert condition
        try expectExpression(
            testCase.condition,
            program.entry.statements[0].expressionStatement.expression.ifExpression.condition.*,
        );

        // Assert consequence "if" branch
        // For these test cases, the consequence and alternative Chunk only allow 1 statement to make testing easier.
        try std.testing.expectEqual(1, program.entry.statements[0].expressionStatement.expression.ifExpression.consequence.statements.len);
        try expectStatement(&testCase.consequence, &program.entry.statements[0].expressionStatement.expression.ifExpression.consequence.statements[0]);
        // Assert alternative "else" branch
        if (testCase.alternative != null) {
            try std.testing.expectEqual(1, program.entry.statements[0].expressionStatement.expression.ifExpression.alternative.?.statements.len);
            try expectStatement(
                &(testCase.alternative.?),
                &program.entry.statements[0].expressionStatement.expression.ifExpression.alternative.?.statements[0],
            );
        }
    }
}

// TODO: this was added to make testing the evaluator early on easier.
// lua doesn't actually allow expression statements, so remove all this and make sure an error
// is raised instead down the line.
//
// MARK: ExpressionStatement Tests
//
test "parse expression statement tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.ExpressionStatement,
    };

    const tests = [_]TestCase{
        .{
            .input = "true",
            .expected = ast.ExpressionStatement{
                .expression = @constCast(&ast.Expression{
                    .boolean = ast.Boolean{ .value = true },
                }),
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();

        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        try expectExpressionStatement(testCase.expected, program.entry.statements[0].expressionStatement);
    }
}

//
// MARK: Local Statement Tests
//
test "parse local tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Local,
    };

    const tests = [_]TestCase{
        .{
            .input = "local foo = 42",
            .expected = ast.Local{
                .name = ast.Identifier{ .value = "foo" },
                .value = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 42 } }),
            },
        },
        .{
            .input = "local x = 1 + 2;",
            .expected = ast.Local{
                .name = ast.Identifier{ .value = "x" },
                .value = @constCast(&ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 1 } }),
                        .operator = ast.Operator.add,
                        .right = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 2 } }),
                    },
                }),
            },
        },
        .{
            .input = "local bar = 'baz';",
            .expected = ast.Local{
                .name = ast.Identifier{ .value = "bar" },
                .value = @constCast(&ast.Expression{ .string = ast.String{ .value = "baz" } }),
            },
        },
        .{
            .input = "local baz = true;",
            .expected = ast.Local{
                .name = ast.Identifier{ .value = "baz" },
                .value = @constCast(&ast.Expression{ .boolean = ast.Boolean{ .value = true } }),
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();

        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        try expectLocal(testCase.expected, program.entry.statements[0].local);
    }
}

//
// MARK: Return Statement Tests
//
test "parse single statement return tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Return,
    };

    const tests = [_]TestCase{
        .{
            .input = "return 42;",
            .expected = ast.Return{
                .value = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 42 } }),
            },
        },
        .{
            .input = "return false;",
            .expected = ast.Return{
                .value = @constCast(&ast.Expression{ .boolean = ast.Boolean{ .value = false } }),
            },
        },
        .{
            .input = "return 'alice';",
            .expected = ast.Return{
                .value = @constCast(&ast.Expression{ .string = ast.String{ .value = "alice" } }),
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();
        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        try expectReturnStatement(testCase.expected, program.entry.statements[0]._return);
    }
}

test "parse chunk tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Chunk,
    };

    const tests = [_]TestCase{
        .{
            .input = "local x = 5;local y = 10;return x + y;",
            .expected = ast.Chunk{
                .statements = &.{
                    ast.Statement{
                        .local = ast.Local{
                            .name = ast.Identifier{ .value = "x" },
                            .value = @constCast(&ast.Expression{
                                .integer = ast.Integer{ .value = 5 },
                            }),
                        },
                    },
                    ast.Statement{
                        .local = ast.Local{
                            .name = ast.Identifier{ .value = "y" },
                            .value = @constCast(&ast.Expression{
                                .integer = ast.Integer{ .value = 10 },
                            }),
                        },
                    },
                    ast.Statement{
                        ._return = ast.Return{
                            .value = @constCast(&ast.Expression{
                                .infixExpression = ast.InfixExpression{
                                    .left = @constCast(&ast.Expression{ .identifier = ast.Identifier{ .value = "x" } }),
                                    .operator = ast.Operator.add,
                                    .right = @constCast(&ast.Expression{ .identifier = ast.Identifier{ .value = "y" } }),
                                },
                            }),
                        },
                    },
                },
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();
        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(testCase.expected.statements.len, program.entry.statements.len);
        try expectChunk(testCase.expected, program.entry);
    }
}

//
// MARK: Function Literal Tests
//
test "parse function tests" {
    var expected_allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer expected_allocator.deinit();

    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Function,
    };

    const tests = [_]TestCase{
        .{
            .input = "function foo() return 5 end",
            .expected = ast.Function{
                .name = ast.Identifier{ .value = "foo" },
                .arguments = &.{},
                .body = @constCast(&ast.Chunk{
                    .statements = &.{
                        ast.Statement{
                            ._return = ast.Return{
                                .value = @constCast(&ast.Expression{
                                    .integer = ast.Integer{ .value = 5 },
                                }),
                            },
                        },
                    },
                }),
            },
        },
        .{
            .input = "function bar(x, y) return x + y end",
            .expected = ast.Function{
                .name = ast.Identifier{ .value = "bar" },
                .arguments = &.{},
                // Can't seem to find the correct syntax for initializing a slice literal in
                // a format like this. It might just not be possible in current zig...
                // below is a list of attempts and their corresponding error messages.
                // .arguments = []ast.Identifier{
                //     ast.Identifier{ .value = "x" },
                //     ast.Identifier{ .value = "y" },
                // },
                // error: type '[]ast.Identifier' does not support array initialization syntax
                // .arguments = [_]ast.Identifier{
                //     ast.Identifier{ .value = "x" },
                //     ast.Identifier{ .value = "y" },
                // },
                // error: array literal requires address-of operator (&) to coerce to slice type '[]ast.Identifier'
                // .arguments = &[_]ast.Identifier{
                //     ast.Identifier{ .value = "x" },
                //     ast.Identifier{ .value = "y" },
                // },
                // error: expected type '[]ast.Identifier', found '*const [2]ast.Identifier'
                // .arguments = .{
                //     ast.Identifier{ .value = "x" },
                //     ast.Identifier{ .value = "y" },
                // },
                // error: type '[]ast.Identifier' does not support array initialization syntax
                // .arguments = &.{
                //     ast.Identifier{ .value = "x" },
                //     ast.Identifier{ .value = "y" },
                // },
                // error: expected type '[]ast.Identifier', found '*const [2]ast.Identifier'
                .body = @constCast(&ast.Chunk{
                    .statements = &.{
                        ast.Statement{
                            ._return = ast.Return{
                                .value = @constCast(&ast.Expression{
                                    .infixExpression = ast.InfixExpression{
                                        .left = @constCast(&ast.Expression{ .identifier = ast.Identifier{ .value = "x" } }),
                                        .operator = ast.Operator.add,
                                        .right = @constCast(&ast.Expression{ .identifier = ast.Identifier{ .value = "y" } }),
                                    },
                                }),
                            },
                        },
                    },
                }),
            },
        },
    };

    for (tests) |testCase| {
        var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
        defer allocator.deinit();

        var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
        defer tokenizer.deinit();
        var parser = try Parser.init(&tokenizer, allocator.allocator());
        const program = try parser.parseProgram();

        try std.testing.expectEqual(1, program.entry.statements.len);
        try expectFunction(testCase.expected, program.entry.statements[0].expressionStatement.expression.function);
    }
}
