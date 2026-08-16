const std = @import("std");
const escapes = @import("escapes");
const state = @import("state");
const builtin = @import("builtin");

pub const full = std.fmt.comptimePrint(
    \\{s} - {s}.
    \\{s}.
    \\VM: {s} ({s})
    \\Version: {s}
    \\Target: {s} ({s}) {s} ({s}) ({s})
    \\Lang: {s} {s}
    \\Supported Features: {s}
    \\Supported addressing modes: {s}
    \\Supported mathmatical expressions: {s}
    \\Supported comparison operations: {s}
    \\Supported string expressions: {s}
    \\Operations accepting expressions: {s}
    \\Operations supporting addressing: {s}
    , .{ name, msg, owner, vmclass, vmexpl, version, os, abi, arch, cpu, opti, lang, langver, 
        features, addr, math, compare, strops, 
        opsack, opaddr }
    );

const str = state.str;

const name: str = "Pebble";
const msg: str = "A VM Language that is both small and secure";
const owner: str = "Product of The Nuovi Orizzonti Company";
const vmclass: str = "Class 1";
const vmexpl: str = "String-storing bytecode machine";
const version: str = "InDev 2026-08-16";
const lang: str = "Zig";
const langver: str = "0.15.2";
const features: str = "New, Escape, Func, Return, End, If, Call";
const math: str = "+, -, /, *, ";
const compare: str = "==, <, >, !=";
const strops: str = "s++, ?=, e?=, s?=, -?=";
const opsack: str = "New, Escape, Func, If, Call";
const addr: str = " , {}, <>, '', \"\"";
const opaddr: str = "New, Escape, Func, If, Call";
const arch = @tagName(builtin.cpu.arch);
const os = @tagName(builtin.os.tag);
const abi = @tagName(builtin.abi);
const cpu: str = builtin.cpu.model.name;
const opti = @tagName(builtin.mode);
