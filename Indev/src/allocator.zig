const std = @import("std");

var arena: ?std.heap.ArenaAllocator = null;

pub fn init() void {
    arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
}

pub fn deinit() void {
    if (arena) |*a| {
        a.deinit();
        arena = null;
    }
}

pub fn alloc() std.mem.Allocator {
    return arena.?.allocator();
}
