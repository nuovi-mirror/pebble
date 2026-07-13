const std = @import("std");
const libs = @import("libs");

pub const escapeFn = *const fn () anyerror!void;
pub const escapesTable = std.StaticStringMap(escapeFn).initComptime(.{
    .{ "Print", libs.Print.run }, // io
    .{ "Input", libs.Input.run }, // io

    .{ "Random", libs.Random.run }, // misc

    .{ "Fread", libs.Fread.run }, // fs
    .{ "Fwrite", libs.Fwrite.run }, // fs
    .{ "Fbwrite", libs.Fbwrite.run }, // fs

    .{ "StrLen", libs.StrLen.run }, // str
    .{ "StrSplitNumL", libs.StrSplitNumL.run }, // str
    .{ "StrSplitNumR", libs.StrSplitNumR.run }, // str
});
