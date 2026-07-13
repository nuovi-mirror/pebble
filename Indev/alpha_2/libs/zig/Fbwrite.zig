const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator for now

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const fileData = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("ARG2") orelse return;
    const fileName = vm.data.get(indirect2) orelse return;
    
    const file = try std.fs.cwd().createFile(fileName, .{});
    defer file.close();

    try file.writeAll(fileData);
}
