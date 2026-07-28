const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const x = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("ARG2") orelse return;
    const y = vm.data.get(indirect2) orelse return;

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    const rx = try std.fmt.parseInt(u8, x, 10) - 1;
    const ry = try std.fmt.parseInt(u8, y, 10) - 1;
    try stdout.print("\x1b[{d};{d}H", .{ ry, rx });
    try stdout.flush();
}
