const std = @import("std");

pub var data: std.StringHashMap([]const u8) = undefined; // define data region
pub var codeTable: std.StringHashMap(usize) = undefined; // define func ptr region
pub var code: std.ArrayList([][]const u8) = undefined; // define func region
