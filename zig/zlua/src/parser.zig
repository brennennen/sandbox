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
        while (self.token.tag != .eof) {
            std.debug.print("   parsing stmt: {d}\n", .{statements.items.len});
            const statement = try self.parseStatement();
            //statements.append(statement) catch return ParserError.InvalidProgram;
            try statements.append(statement);
            try self.next();
        }

        return ast.Program{ .statements = statements };
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
                if (std.fmt.parseInt(i64, token, 10)) |number| {
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

    fn parseStringLiteral(self: *Parser) ParserError!ast.StringLiteral {
        if (self.token.tag == .string_literal) {
            const stringToken = self.token.getValue(self.tokenizer.buffer);
            // Strip off the quotation marks.
            return ast.StringLiteral{ .value = stringToken[1 .. stringToken.len - 1] };
        } else {
            return ParserError.UnexpectedToken;
        }
    }

    // TODO: parse grouped expression? block expression?
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

        const consequence = try self.allocator.create(ast.Block);
        consequence.* = try self.parseBlockStatement();
        var ifExpression = ast.IfExpression{
            .condition = condition,
            .consequence = consequence,
            .alternative = null,
        };

        if (self.token.tag == .keyword_end) {
            try self.next();
            return ifExpression;
        }

        // TODO: elseif

        if (self.token.tag == .keyword_else) {
            try self.next();
            const alternative = try self.allocator.create(ast.Block);
            alternative.* = try self.parseBlockStatement();
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

    //
    // MARK: Statement Parsing
    //
    pub fn parseStatement(self: *Parser) ParserError!ast.Statement {
        std.debug.print("parseStatement: tok: {}\n", .{self.token.tag});
        switch (self.token.tag) {
            .keyword_local => {
                return ast.Statement{ .local = try self.parseLocalStatement() };
            },
            .keyword_return => {
                return ast.Statement{ ._return = try self.parseReturnStatement() };
            },
            else => {
                return ast.Statement{ .expression = try self.parseExpressionStatement() };
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

    pub fn parseExpressionStatement(self: *Parser) ParserError!ast.Expression {
        const expression = try self.parseExpression(.lowest);
        if (self.token.tag == .semicolon) {
            try self.next();
        }
        return expression;
    }

    pub fn isTokenBlockEnd(token: Token) bool {
        if (token.tag == .keyword_end or
            token.tag == .keyword_elseif or
            token.tag == .keyword_else)
        {
            return true;
        } else {
            return false;
        }
    }

    pub fn parseBlockStatement(self: *Parser) ParserError!ast.Block {
        std.debug.print("parseBlockStatement: tok: {}\n", .{self.token.tag});
        var statements = std.ArrayList(ast.Statement).init(self.allocator);

        while (!isTokenBlockEnd(self.token)) {
            try statements.append(try parseStatement(self));
        }
        return ast.Block{ .statements = statements };
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
                return ast.Expression{ .stringLiteral = try self.parseStringLiteral() };
            },
            .keyword_if => {
                return ast.Expression{ .ifExpression = try self.parseIfExpression() };
            },
            // .keyword_function => {
            //     return ast.Expression{ .function = try self.parseFunctionLiteral}
            // },
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

//
// MARK: TESTS
//

pub const ParserTestError = ParserError || error{
    TestExpectedEqual,
    UnexpectedStatement,
    UnexpectedExpression,
};

fn expectStatement(expected: *const ast.Statement, actual: *const ast.Statement) ParserTestError!void {
    switch (expected.*) {
        //.local => |local| try expectLocalStatement(&local, actual),
        .local => |expectedLocal| {
            switch (actual.*) {
                .local => |actualLocal| {
                    try expectLocalStatement(&expectedLocal, &actualLocal);
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
                    try expectReturnStatement(&expectedReturn, &actualReturn);
                },
                else => {
                    std.debug.print("expectStatement: unknown actual statement type: {}\n", .{actual});
                    return error.UnexpectedStatement;
                },
            }
        },
        .expression => |expectedExpression| {
            switch (actual.*) {
                .expression => |actualExpression| {
                    try expectExpression(&expectedExpression, &actualExpression);
                },
                else => {
                    std.debug.print("expectStatement: unknown actual statement type: {}\n", .{actual});
                    return error.UnexpectedStatement;
                },
            }
        },
        .block => |expectedBlock| {
            switch (actual.*) {
                .block => |actualBlock| {
                    try expectBlock(&expectedBlock, &actualBlock);
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

fn expectLocalStatement(expected: *const ast.Local, actual: *const ast.Local) ParserTestError!void {
    try expectIdentifier(&expected.*.name, &actual.*.name);
    try expectExpression(expected.*.value, actual.*.value);
}

fn expectReturnStatement(expected: *const ast.Return, actual: *const ast.Return) ParserTestError!void {
    try expectExpression(expected.*.value, actual.*.value);
}

fn expectIdentifier(expected: *const ast.Identifier, actual: *const ast.Identifier) ParserTestError!void {
    try std.testing.expectEqualStrings(expected.*.value, actual.*.value);
}

fn expectExpression(expected: *const ast.Expression, actual: *const ast.Expression) ParserTestError!void {
    switch (expected.*) {
        .integer => |expectedInteger| {
            switch (actual.*) {
                .integer => |actualInteger| {
                    try expectInteger(&expectedInteger, &actualInteger);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .stringLiteral => |expectedString| {
            switch (actual.*) {
                .stringLiteral => |actualString| {
                    try expectStringLiteral(&expectedString, &actualString);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .boolean => |expectedBool| {
            switch (actual.*) {
                .boolean => |actualBool| {
                    try expectBoolean(&expectedBool, &actualBool);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .prefixExpression => |expectedPrefixExpression| {
            switch (actual.*) {
                .prefixExpression => |actualPrefixExpression| {
                    try expectPrefixExpression(&expectedPrefixExpression, &actualPrefixExpression);
                },
                else => {
                    return error.UnexpectedExpression;
                },
            }
        },
        .infixExpression => |expectedInfixExpression| {
            switch (actual.*) {
                .infixExpression => |actualInfixExpression| {
                    try expectInfixExpression(&expectedInfixExpression, &actualInfixExpression);
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

fn expectBlock(expected: *const ast.Block, actual: *const ast.Block) ParserTestError!void {
    try std.testing.expectEqual(
        expected.statements.items.len,
        actual.statements.items.len,
    );

    // for (expected.statements.items, actual.statements.items) |expStmt, actStmt| {
    //     try expectStatement(expStmt, actStmt);
    // }
}

fn expectPrefixExpression(expected: *const ast.PrefixExpression, actual: *const ast.PrefixExpression) ParserTestError!void {
    try std.testing.expectEqual(expected.operator, actual.operator);
    try expectExpression(expected.right, actual.right);
}

fn expectInfixExpression(expected: *const ast.InfixExpression, actual: *const ast.InfixExpression) ParserTestError!void {
    try expectExpression(expected.left, actual.left);
    try std.testing.expectEqual(expected.operator, actual.operator);
    try expectExpression(expected.right, actual.right);
}

fn expectInteger(expected: *const ast.Integer, actual: *const ast.Integer) ParserTestError!void {
    try std.testing.expectEqual(expected.*.value, actual.*.value);
}

fn expectStringLiteral(expected: *const ast.StringLiteral, actual: *const ast.StringLiteral) ParserTestError!void {
    try std.testing.expectEqualStrings(expected.value, actual.value);
}

fn expectBoolean(expected: *const ast.Boolean, actual: *const ast.Boolean) ParserTestError!void {
    try std.testing.expectEqual(expected.value, actual.value);
}

// Test the expression pratt parsing and generation of the AST.
test "parse single statement arithmetic expression tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Statement,
    };

    const tests = [_]TestCase{
        .{
            .input = "1 + 2",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 1 },
                        }),
                        .operator = ast.Operator.add,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 2 },
                        }),
                    },
                },
            },
        },
        .{
            .input = "34 - 567",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 34 },
                        }),
                        .operator = ast.Operator.subtract,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 567 },
                        }),
                    },
                },
            },
        },
        .{
            .input = "89 * 0",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 89 },
                        }),
                        .operator = ast.Operator.multiply,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 0 },
                        }),
                    },
                },
            },
        },
        .{
            .input = "12 / 3",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 12 },
                        }),
                        .operator = ast.Operator.divide,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 3 },
                        }),
                    },
                },
            },
        },
        .{
            .input = "140 // 20",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
                        .left = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 140 },
                        }),
                        .operator = ast.Operator.divide_floor,
                        .right = @constCast(&ast.Expression{
                            .integer = ast.Integer{ .value = 20 },
                        }),
                    },
                },
            },
        },
        .{
            .input = "5 + 6 - 7",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
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
            },
        },
        .{
            .input = "-1 + 2",
            .expected = ast.Statement{
                .expression = ast.Expression{
                    .infixExpression = ast.InfixExpression{
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

        try std.testing.expectEqual(1, program.statements.items.len);
        try expectStatement(&testCase.expected, &program.statements.items[0]);
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

        try std.testing.expectEqual(1, program.statements.items.len);
        const actual = try std.fmt.allocPrint(allocator.allocator(), "{}", .{
            program.statements.items[0],
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

        try std.testing.expectEqual(1, program.statements.items.len);
        const actual = try std.fmt.allocPrint(allocator.allocator(), "{}", .{
            program.statements.items[0],
        });
        try std.testing.expectEqualStrings(testCase.expected, actual);
    }
}

//
// MARK: IfExpression Tests
//
test "adhoc if test 1" {
    const input = "if 1 == 1 then return 1 end";

    var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer allocator.deinit();

    var tokenizer = Tokenizer.init(allocator.allocator(), input);
    defer tokenizer.deinit();
    var parser = try Parser.init(&tokenizer, allocator.allocator());
    var program = try parser.parseProgram();
    defer program.deinit();
    std.debug.print("{}\n", .{program});
    try std.testing.expectEqual(1, program.statements.items.len);
    // TODO: check condition
    // TODO: check consequence
}

test "adhoc if test 2" {
    const input = "if 1 == 2 then return 1 else return 5 end";

    var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer allocator.deinit();

    var tokenizer = Tokenizer.init(allocator.allocator(), input);
    defer tokenizer.deinit();
    var parser = try Parser.init(&tokenizer, allocator.allocator());
    var program = try parser.parseProgram();
    defer program.deinit();
    std.debug.print("{}\n", .{program});
    try std.testing.expectEqual(1, program.statements.items.len);
    // expectExpression(
    //     @constCast(&ast.Expression{
    //         .infixExpression = ast.InfixExpression{
    //             .left = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 1 } }),
    //             .operator = ast.Operator.equal,
    //             .right = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 2 } }),
    //         }
    //     }), program.statments.items[0].

    // TODO: check condition
    // TODO: check consequence
}

// test "parse IfExpression tests" {
//     var expected_allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
//     defer expected_allocator.deinit();

//     const TestCase = struct {
//         input: [:0]const u8,
//         statement: ast.Statement,
//     };

//     const tests = [_]TestCase{
//         .{
//             .input = "if (1 == 2) then return 1 end",
//             .statement = ast.Statement{
//                 .expression = ast.Expression{
//                     .ifExpression = ast.IfExpression{
//                         .condition = @constCast(&ast.Expression{
//                             .infixExpression = ast.InfixExpression{
//                                 .left = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 1 } }),
//                                 .operator = ast.Operator.equal,
//                                 .right = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 1 } }),
//                             },
//                         }),
//                         .consequence = @constCast(&ast.Block{
//                             .statements = std.ArrayList(ast.Statement).init(expected_allocator.allocator()),
//                         }),
//                         .alternative = @constCast(&ast.Block{
//                             .statements = std.ArrayList(ast.Statement).init(expected_allocator.allocator()),
//                         }),
//                     },
//                 },
//             },
//         },
//     };

//     for (tests) |testCase| {
//         var allocator = std.heap.ArenaAllocator.init(std.testing.allocator);
//         defer allocator.deinit();

//         var tokenizer = Tokenizer.init(std.testing.allocator, testCase.input);
//         defer tokenizer.deinit();
//         var parser = try Parser.init(&tokenizer, allocator.allocator());
//         const program = try parser.parseProgram();

//         try std.testing.expectEqual(1, program.statements.items.len);
//         const actual = try std.fmt.allocPrint(allocator.allocator(), "{}", .{
//             program.statements.items[0],
//         });
//         try std.testing.expectEqualStrings(testCase.expected, actual);
//     }
// }

//
// MARK: Local Statement Tests
//
test "parse single statement local tests" {
    const TestCase = struct {
        input: [:0]const u8,
        statement: ast.Statement,
    };

    const tests = [_]TestCase{
        .{
            .input = "local foo = 42",
            .statement = ast.Statement{
                .local = ast.Local{
                    .name = ast.Identifier{ .value = "foo" },
                    .value = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 42 } }),
                },
            },
        },
        .{
            .input = "local x = 1 + 2;",
            .statement = ast.Statement{
                .local = ast.Local{
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
        },
        .{
            .input = "local bar = 'baz';",
            .statement = ast.Statement{
                .local = ast.Local{
                    .name = ast.Identifier{ .value = "bar" },
                    .value = @constCast(&ast.Expression{ .stringLiteral = ast.StringLiteral{ .value = "baz" } }),
                },
            },
        },
        .{
            .input = "local baz = true;",
            .statement = ast.Statement{
                .local = ast.Local{
                    .name = ast.Identifier{ .value = "baz" },
                    .value = @constCast(&ast.Expression{ .boolean = ast.Boolean{ .value = true } }),
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

        try std.testing.expectEqual(1, program.statements.items.len);
        try expectStatement(&testCase.statement, &program.statements.items[0]);
    }
}

//
// MARK: Return Statement Tests
//
test "parse single statement return tests" {
    const TestCase = struct {
        input: [:0]const u8,
        expected: ast.Statement,
    };

    const tests = [_]TestCase{
        .{
            .input = "return 42;",
            .expected = ast.Statement{
                ._return = ast.Return{
                    .value = @constCast(&ast.Expression{ .integer = ast.Integer{ .value = 42 } }),
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

        try std.testing.expectEqual(1, program.statements.items.len);
        try expectStatement(&testCase.expected, &program.statements.items[0]);
    }
}
