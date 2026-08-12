const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.str.find_ARG0") orelse return;
    const str = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("__Escape_std.str.find_ARG1") orelse return;
    const target = vm.data.get(indirect2) orelse return;

    var counter: usize = 0;

    for (str) |current| {
        if (current == target[0]) {
            counter += 1;
            break;
        }
        counter += 1;
    }

    const itemAsString = try std.fmt.allocPrint(mem.alloc(), "{d}", .{counter});
    try vm.data.put("__Escape_std.str.find_RET0", itemAsString);
}
