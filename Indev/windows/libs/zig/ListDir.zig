const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator for now

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const dirName = vm.data.get(indirect) orelse return;
    
    var dir = try std.fs.cwd().openDir(dirName, .{ .iterate = true });
    defer dir.close();

    var iterator = dir.iterate();

    var list: std.ArrayList(u8) = .empty;
    defer list.deinit(allocator);

    while (try iterator.next()) |entry| {
        try list.appendSlice(allocator, entry.name);
        try list.append(allocator, '\n');
    }

    const result = try list.toOwnedSlice(allocator);

    try vm.data.put("dirContents", result);
}
