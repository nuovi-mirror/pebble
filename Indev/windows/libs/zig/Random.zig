const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator
const rand = std.crypto.random;

pub fn run() !void {
    const numb = rand.int(u16);
    const str = try std.fmt.allocPrint(
        allocator,
        "{}",
        .{numb},
    );
    try vm.data.put("random", str);
}
