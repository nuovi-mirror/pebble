const std = @import("std");
const libs = @import("libs");

pub const escapeFn = *const fn () anyerror!void;
pub const escapesTable = std.StaticStringMap(escapeFn).initComptime(.{
    .{ "Print", libs.Print.run }, // io
    .{ "PrintLn", libs.PrintLn.run }, // io
    .{ "Input", libs.Input.run }, // io

    .{ "Random", libs.Random.run }, // misc
    .{ "Time", libs.Time.run }, // misc

    .{ "Fread", libs.Fread.run }, // fs
    .{ "Fwrite", libs.Fwrite.run }, // fs
    .{ "Fbwrite", libs.Fbwrite.run }, // fs
    .{ "ListDir", libs.ListDir.run }, // fs

    .{ "StrLen", libs.StrLen.run }, // str
    .{ "StrSplitNumL", libs.StrSplitNumL.run }, // str
    .{ "StrSplitNumR", libs.StrSplitNumR.run }, // str

    .{ "SpawnProc", libs.SpawnProc.run }, // proc
    .{ "WaitPid", libs.WaitPid.run }, // proc
    .{ "Exit", libs.Exit.run }, // proc
});
