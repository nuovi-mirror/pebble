const std = @import("std");
const escapes = @import("escapes");
const state = @import("state");

pub const full = std.fmt.comptimePrint(
    \\{s} - {s}.
    \\{s}.
    \\VM: {s} ({s})
    \\Version: {s}
    \\Lang: {s} {s}
    \\Supported Features: {s}
    \\Supported mathmatical expressions: {s}
    \\Supported comparison operations: {s}
    \\Supported string expressions: {s}
    \\Operations accepting expressions: {s}
    \\Supported addressing modes: {s}
    \\Operations supporting addressing: {s}
    , .{ name, msg, owner, vmclass, vmexpl, version, lang, langver, features, math, compare, strops, 
        opsack, addr, opaddr }
    );

const str = state.str;

const name: str = "Pebble";
const msg: str = "A VM Language that is both small and secure";
const owner: str = "Product of The Nuovi Orizzonti Company";
const vmclass: str = "Class 1";
const vmexpl: str = "Array-storing bytecode machine";
const version: str = "InDev 2026-08-8 1";
const lang: str = "Zig";
const langver: str = "0.15.2";
const features: str = "New, Escape, Func, Return, End, If, Call";
const math: str = "+, -, /, *, ";
const compare: str = "==, <, >, !=";
const strops: str = "s++, ?=, e?=, s?=, -?=";
const opsack: str = "New, Escape, Func, If, Call";
const addr: str = " , {}, <>, '', \"\"";
const opaddr: str = "New, Escape, Func, If, Call";
