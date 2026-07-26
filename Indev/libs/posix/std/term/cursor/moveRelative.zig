const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const dir = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("ARG2") orelse return;
    const num = vm.data.get(indirect2) orelse return;

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    const escape: u8 = blk: {
        if (std.mem.eql(u8, dir, "up")) break :blk 'A';
        if (std.mem.eql(u8, dir, "down")) break :blk 'B';
        if (std.mem.eql(u8, dir, "left")) break :blk 'C';
        if (std.mem.eql(u8, dir, "right")) break :blk 'D';

        return error.InvalidDirection;
    };

    const count = try std.fmt.parseInt(i32, num, 10);
    try stdout.print("\x1b[{d}{c}", .{ count, escape });
    try stdout.flush();
}

