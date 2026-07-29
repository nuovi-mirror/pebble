const std = @import("std");
const build = @import("build_options");

var arena: ?std.heap.ArenaAllocator = null;
var debug = std.heap.DebugAllocator(.{}){};

pub fn init() void {
    if (build.VMDEBUG) {
        arena = std.heap.ArenaAllocator.init(debug.allocator());
    } else { 
        arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    }

    _ = arena.?.allocator().alloc(u8, 1024 * 1024 * 32) catch {};
}

pub fn queryCapacity() usize {
    if (build.VMDEBUG) {
        return arena.?.queryCapacity();
    } else {
        return 0;
    }
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
