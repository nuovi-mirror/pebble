const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const fileRead = vm.data.get(indirect) orelse return;

    const data = try std.fs.cwd().readFileAlloc(
        mem.alloc(),
        fileRead,
        512 * 1024 * 1024,
    );

    try vm.data.put("fileData", data);
}
