const std = @import("std");
const allocator = std.heap.page_allocator;
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const str = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("ARG2") orelse return;
    const target = vm.data.get(indirect2) orelse return;

    var counter: usize = 0;

    for (str) |current| {
        if (current == target[0]) {
            counter += 1;
            break;
        }
        counter += 1;
    }

    const itemAsString = try std.fmt.allocPrint(allocator, "{d}", .{counter});
    try vm.data.put("entry", itemAsString);
}
