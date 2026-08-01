const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.io.printLn_ARG0") orelse return;
    const msg = vm.data.get(indirect) orelse return;

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    try stdout.print("{s}", .{msg});
    try stdout.flush();
}

