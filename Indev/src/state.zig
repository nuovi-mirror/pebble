const std = @import("std");

pub var data: std.StringHashMap(str) = undefined; // define data region
pub var codeTable: std.StringHashMap(word) = undefined; // define func ptr region
pub var code: std.ArrayList(dstr) = undefined; // define func region

pub const word = u32; // word
pub const hword = u16; // half word
pub const byte = u8; // byte
pub const sword = i32; // signed word
pub const shword = i16; // signed half word
pub const sbyte = i8; // signed byte
pub const spf = f32; // single precision floating point
pub const float = spf; // default floating point value
pub const str = []const u8; // string
pub const wstr = []const []const u8; // wrapped string
pub const dstr = [][]const u8; // double string
pub const mstr = []u8; // mutable string
pub const wmstr = []const []u8; // wrapped mutable string
pub const dmstr = [][]u8; // double mutable string
