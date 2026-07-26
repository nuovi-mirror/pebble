const std = @import("std");

var print_buffer: [4096]u8 = undefined;
var print_writer = std.fs.File.stdout().writer(&print_buffer);

pub const exit = std.process.exit;
pub fn print(comptime fmt: []const u8, args: anytype) void {
    print_writer.interface.print(fmt, args) catch {};
    print_writer.interface.flush() catch {};
}
