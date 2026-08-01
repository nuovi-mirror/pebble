const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.io.fs.dir.change_ARG1") orelse return;
    const dir = vm.data.get(indirect) orelse return;
    try std.posix.chdir(dir);
}
