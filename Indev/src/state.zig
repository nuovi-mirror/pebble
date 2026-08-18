const std = @import("std");
const build = @import("build_options");

pub var data: std.StringHashMap(str) = undefined; // define data region
pub var codeTable: std.StringHashMap(word) = undefined; // define func ptr region
pub var code: std.ArrayList(dstr) = undefined; // define func region

pub const word = if (build.native) usize else u32; // host size unless
pub const sword = if (build.native) isize else i32; // host size unless
pub const float = if (build.native) f64 else f32; // host size unless

pub const hword = if (build.native) 
    if (@bitSizeOf(usize) == 64) u32 
    else u16 else u16; // half word

pub const shword = if (build.native) 
    if (@bitSizeOf(isize) == 64) i32 
    else i16 else i16; // signed half word

pub const quword = if (build.native)
    if (@bitSizeOf(usize) == 64) u16
    else u8 else u8; // quarter word

pub const squword = if (build.native)
    if (@bitSizeOf(isize) == 64) i16
    else i8 else i8; // signed quarter word

pub const dword = if (build.double) u64; // double word
pub const sdword = if (build.double) i64; // signed double word
pub const dpf = if (build.double) f64; // double precision float

pub const fword = u32; // fixed word
pub const sfword = i32; // signed fixed word
pub const hfword = u16; // half fixed word
pub const shfword = i16; // signed half fixed word

pub const spf = f32; // single precision floating point
pub const byte = u8; // byte
pub const sbyte = i8; // signed byte
pub const str = []const u8; // string
pub const wstr = []const []const u8; // wrapped string
pub const dstr = [][]const u8; // double string
pub const mstr = []u8; // mutable string
pub const wmstr = []const []u8; // wrapped mutable string
pub const dmstr = [][]u8; // double mutable string
