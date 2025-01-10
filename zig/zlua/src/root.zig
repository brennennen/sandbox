const std = @import("std");
const testing = std.testing;

test {
    _ = @import("./tokenizer.zig");
    _ = @import("./parser.zig");
    //_ = @import("./evaluator.zig");
}
