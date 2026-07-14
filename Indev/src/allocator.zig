const std = @import("std");

var arena: std.heap.ArenaAllocator = undefined;

pub fn init() void {
    arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
}

pub fn deinit() void {
    arena.deinit();
}

pub fn alloc() std.mem.Allocator {
    return arena.allocator();
}
