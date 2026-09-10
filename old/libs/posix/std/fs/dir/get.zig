const std = @import("std");
const vm = @import("state");

const allocator = std.heap.page_allocator;

pub fn run() !void {
    var buffer: [std.fs.max_path_bytes]u8 = undefined;

    const cwd = try std.posix.getcwd(&buffer);
    const result = try allocator.dupe(u8, cwd);

    try vm.data.put("__Escape_std.fs.dir.get_RET0", result);
}
