const std = @import("std");

pub const full = std.fmt.comptimePrint(
    \\{s} - {s}.
    \\VM: {s} ({s})
    \\Version: {s}
    \\Lang: {s} {s}
    \\Supported Features: {s}
    \\Supported mathmatical expressions: {s}
    \\Supported comparison operations: {s}
    \\Supported string expressions: {s}
    \\Operations accepting expressions: {s}
    , .{ name, msg, vmclass, vmexpl, version, lang, langver, features, math, compare, str, opsack }
    );

const name: []const u8 = "Pebble";
const msg: []const u8 = "A VM Language that is both small and secure";
const vmclass: []const u8 = "Class 1";
const vmexpl: []const u8 = "Array-storing bytecode machine";
const version: []const u8 = "Alpha 3 InDev 2026-07-17 1";
const lang: []const u8 = "Zig";
const langver: []const u8 = "0.15.2";
const features: []const u8 = "New, Escape, Func, Return, End, If, Call";
const math: []const u8 = "+, -, /, *, ";
const compare: []const u8 = "==, <, >, !=";
const str: []const u8 = "s++, ?=, e?=, s?=, -?=";
const opsack: []const u8 = "New, If";
