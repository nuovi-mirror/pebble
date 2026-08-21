const std = @import("std");
const build = @import("build_options");

var persistent_arena: ?std.heap.ArenaAllocator = null;
var scratch_arena: ?std.heap.ArenaAllocator = null;
var temp_arena: ?std.heap.ArenaAllocator = null;
var debug = std.heap.DebugAllocator(.{}){};

fn backing() std.mem.Allocator {
    return if (build.debug) debug.allocator() else std.heap.c_allocator;
}

pub fn init() void {
    persistent_arena = std.heap.ArenaAllocator.init(backing());
    scratch_arena = std.heap.ArenaAllocator.init(backing());
    temp_arena = std.heap.ArenaAllocator.init(backing());

    _ = persistent_arena.?.allocator().alloc(u8, 8 * 1024 * 1024) catch {}; // 8MB
    _ = scratch_arena.?.allocator().alloc(u8, 2 * 1024 * 1024) catch {}; // 2MB
    _ = temp_arena.?.allocator().alloc(u8, 256 * 1024) catch {}; // 256KB
}

pub fn deinit() void {
    if (persistent_arena) |*a| { a.deinit(); persistent_arena = null; }
    if (scratch_arena) |*a| { a.deinit(); scratch_arena = null; }
    if (temp_arena) |*a| { a.deinit(); temp_arena = null; }
}

pub fn persistent() std.mem.Allocator { return persistent_arena.?.allocator(); }
pub fn scratch() std.mem.Allocator { return scratch_arena.?.allocator(); }
pub fn temp() std.mem.Allocator { return temp_arena.?.allocator(); }

pub fn resetScratch() void {
    if (scratch_arena) |*a| _ = a.reset(.retain_capacity);
}

pub fn resetTemp() void {
    if (temp_arena) |*a| _ = a.reset(.retain_capacity);
}

pub fn queryCapacity() usize {
    if (!build.debug) return 0;
    return persistent_arena.?.queryCapacity() +
        scratch_arena.?.queryCapacity() +
        temp_arena.?.queryCapacity();
}

// legacy compat
pub fn alloc() std.mem.Allocator {
    return persistent();
}
