const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator for now

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const fileRead = vm.data.get(indirect) orelse return;

    const data = try std.fs.cwd().readFileAlloc(
        allocator,
        fileRead,
        512 * 1024 * 1024,
    );

    try vm.data.put("fileData", data);
}
