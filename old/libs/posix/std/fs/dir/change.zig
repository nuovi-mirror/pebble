const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.fs.dir.change_ARG0") orelse return;
    const dir = vm.data.get(indirect) orelse return;
    try std.posix.chdir(dir);
}
