const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.io.fs.dir.list_ARG1") orelse return;
    const dirName = vm.data.get(indirect) orelse return;
    
    var dir = try std.fs.cwd().openDir(dirName, .{ .iterate = true });
    defer dir.close();

    var iterator = dir.iterate();

    var list: std.ArrayList(u8) = .empty;
    defer list.deinit(mem.alloc());

    while (try iterator.next()) |entry| {
        try list.appendSlice(mem.alloc(), entry.name);
        try list.append(mem.alloc(), '\n');
    }

    const result = try list.toOwnedSlice(mem.alloc());

    try vm.data.put("__Escape_std.io.fs.dir.get_RE0", result);
}
