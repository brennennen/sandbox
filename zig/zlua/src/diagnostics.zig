const std = @import("std");

pub const Diagnostics = struct {
    pub const Tag = enum {
        todo,
        error_directive,
    };
};
