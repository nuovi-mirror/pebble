const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const str = vm.data.get(indirect) orelse return;
    const len = str.len;
    const strLen = try std.fmt.allocPrint(mem.alloc(), "{d}", .{len});
    try vm.data.put("len", strLen);
}
