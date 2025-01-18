const std = @import("std");

pub const Object = union(enum) {
    nil: Nil,
    //number: Number,
    integer: Integer,
    boolean: Boolean,
    _return: Return,

    pub fn newInteger(allocator: std.mem.Allocator, value: i64) !*Object {
        const object = try allocator.create(Object);
        object.* = Object{ .integer = Integer{ .value = value } };
        return object;
    }
};

pub const Nil = struct {
    pub fn format(_: Nil, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("nil", .{});
    }
};

// need to represent floats and i64s
// maybe do a union?
pub const Number = union(enum) {
    integer: i64,
    float: f64,

    pub fn format(self: Number, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const Boolean = struct {
    value: bool,

    pub fn format(self: Boolean, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const Integer = struct {
    value: i64,

    pub fn format(self: Integer, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const Return = struct {
    value: *Object,
};
