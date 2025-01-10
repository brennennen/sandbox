const std = @import("std");
const object = @import("object.zig");

pub const Builtin = enum {
    print,

    pub fn call(self: Builtin) void {
        switch (self) {
            .print => try print(),
        }
    }
};

pub fn print() void {
    std.debug.print("todo!");
}
