const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    try stdout.print("\x1b[K", .{});
    try stdout.flush();
}
