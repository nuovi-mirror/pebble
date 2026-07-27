const std = @import("std");

var arena: ?std.heap.ArenaAllocator = null;    
var debug = std.heap.DebugAllocator(.{}){};

pub fn init() void {
    arena = std.heap.ArenaAllocator.init(debug.allocator());
}

pub fn queryCapacity() usize {
    return arena.?.queryCapacity();
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
