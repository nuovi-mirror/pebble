const std = @import("std");
const state = @import("state");
const stdlib = @embedFile("stdlib.rock");

// types
const word = state.word;
const hword = state.hword;
const byte = state.byte;
const sword = state.sword;
const shword = state.shword;
const sbyte = state.sbyte;
const spf = state.spf;
const float = state.float;
const str = state.str;
const wstr = state.wstr;
const dstr = state.dstr;
const mstr = state.mstr;
const wmstr = state.wmstr;
const dmstr = state.dmstr; 

fn readFile(name: str) !str {
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    const allocator = arena.allocator();          

    const data = try std.fs.cwd().readFileAlloc(
        allocator,
        name,
        1024 * 1024 * 512,
    );      

    const code = try std.mem.concat(
        allocator,
        u8,
        &.{ stdlib, data }, // inject stdlib data
    );              

    defer allocator.free(data);
    defer allocator.free(code);
    defer arena.deinit();

    return code;
}

pub fn main() !void {
    var _args = std.process.args();
    _ = _args.next(); // skip first
    const arg = _args.next() orelse return;

    const code = readFile(arg);
}
