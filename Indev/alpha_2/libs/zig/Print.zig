const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const msg = vm.data.get(indirect) orelse return;
    std.debug.print("{s}\n", .{msg});
}
