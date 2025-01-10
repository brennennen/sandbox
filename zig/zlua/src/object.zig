const std = @import("std");

pub const Object = union(enum) {
    nil: Nil,
    boolean: Boolean,
    _return: Return,
};

pub const Nil = struct {
    pub fn format(_: Nil, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("nil", .{});
    }
};

// need to represent floats and i64s
pub const Number = struct { value: i64 };

pub const Boolean = struct {
    value: bool,

    pub fn format(self: Boolean, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const Return = struct {
    value: *Object,
};
