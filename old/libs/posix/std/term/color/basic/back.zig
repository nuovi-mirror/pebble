const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.term.color.basic.fore_ARG0") 
        orelse return;
    
    const color = vm.data.get(indirect) orelse return;

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    const code: u8 = blk: {
        if (std.mem.eql(u8, color, "black")) break :blk 40;
        if (std.mem.eql(u8, color, "red")) break :blk 41;
        if (std.mem.eql(u8, color, "green")) break :blk 42;
        if (std.mem.eql(u8, color, "yellow")) break :blk 43;
        if (std.mem.eql(u8, color, "blue")) break :blk 44;
        if (std.mem.eql(u8, color, "magenta")) break :blk 45;
        if (std.mem.eql(u8, color, "cyan")) break :blk 46;
        if (std.mem.eql(u8, color, "white")) break :blk 47;

        return error.InvalidColor;
    };

    try stdout.print("\x1b[{d}m", .{code});
    try stdout.flush();
}
