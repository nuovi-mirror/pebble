const std = @import("std");

var print_buffer: [4096]u8 = undefined;
var print_writer = std.fs.File.stdout().writer(&print_buffer);

pub const exit = std.process.exit;
pub fn print(comptime fmt: []const u8, args: anytype) void {
    print_writer.interface.print(fmt, args) catch {};
    print_writer.interface.flush() catch {};
}

pub fn input() []const u8 {
    var input_buffer: [1024]u8 = undefined;
    var input_reader = std.fs.File.stdin().reader(&input_buffer);
            
    const stdin = &input_reader.interface;
    return stdin.takeDelimiterExclusive('\n') catch |err| switch (err) {
        error.EndOfStream => "",
        error.ReadFailed => "",
        error.StreamTooLong => "",
    };
}
