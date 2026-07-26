const std = @import("std");
const allocator = std.heap.page_allocator;
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const str = vm.data.get(indirect) orelse return;
    const len = str.len;
    const strLen = try std.fmt.allocPrint(allocator, "{d}", .{len});
    try vm.data.put("len", strLen);
}
