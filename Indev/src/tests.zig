const std = @import("std");
const mem = @import("allocator");
pub const tests = @embedFile("embedded/tests.pebble");
pub const dance = @embedFile("embedded/dance.pebble");

pub fn read() []const u8 {
    return tests;
}
