const std = @import("std");
const mem = @import("allocator");
pub const docs = @embedFile("embedded/docs.txt");

pub fn read() []const u8 {
    return docs;
}
