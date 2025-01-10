const std = @import("std");

pub const Environment = struct {
    allocator: std.mem.Allocator,
    outer: ?*Environment,

    pub fn init(allocator: std.mem.Allocator) Environment {
        return .{
            .allocator = allocator,
            .outer = null,
        };
    }
};
