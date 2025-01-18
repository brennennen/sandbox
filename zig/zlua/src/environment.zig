const std = @import("std");
const Object = @import("./object.zig").Object;

pub const EnvironmentError = error{OutOfMemory} || error{};

pub const Environment = struct {
    allocator: std.mem.Allocator,
    outer: ?*Environment,
    store: std.StringHashMap(*Object),

    pub fn init(allocator: std.mem.Allocator) Environment {
        return .{
            .allocator = allocator,
            .outer = null,
            .store = std.StringHashMap(*Object).init(allocator),
        };
    }

    pub fn get(self: *Environment, key: []const u8) ?*Object {
        if (self.store.get(key)) |val| {
            return val;
        }

        if (self.outer) |outer| {
            return outer.get(key);
        }

        return null;
    }

    pub fn set(self: *Environment, key: []const u8, value: *Object) EnvironmentError!void {
        try self.store.put(key, value);
    }
};
