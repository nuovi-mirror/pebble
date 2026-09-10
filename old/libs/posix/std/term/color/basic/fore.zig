const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.term.color.basic.back_ARG0") 
        orelse return;
    const color = vm.data.get(indirect) orelse return;

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    const code: u8 = blk: {
        if (std.mem.eql(u8, color, "black")) break :blk 30;
        if (std.mem.eql(u8, color, "red")) break :blk 31;
        if (std.mem.eql(u8, color, "green")) break :blk 32;
        if (std.mem.eql(u8, color, "yellow")) break :blk 33;
        if (std.mem.eql(u8, color, "blue")) break :blk 34;
        if (std.mem.eql(u8, color, "magenta")) break :blk 35;
        if (std.mem.eql(u8, color, "cyan")) break :blk 36;
        if (std.mem.eql(u8, color, "white")) break :blk 37;

        return error.InvalidColor;
    };

    try stdout.print("\x1b[{d}m", .{code});
    try stdout.flush();
}
