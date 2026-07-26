const std = @import("std");
const allocator = std.heap.page_allocator;
const vm = @import("state");

pub fn run() !void {
    var stdin_buffer: [1024]u8 = undefined;
    var stdin_reader = std.fs.File.stdin().reader(&stdin_buffer);
    
    const stdin = &stdin_reader.interface;
    
    const input = try stdin.takeDelimiterExclusive('\n');
    const copy = try allocator.dupe(u8, input);
    try vm.data.put("input", copy);
}
