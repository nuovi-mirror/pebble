const std = @import("std");
const build = @import("build_options");
const mem = @import("allocator");

pub var data: std.StringHashMap(Value) = undefined; // define data region
pub var codeTable: std.StringHashMap(word) = undefined; // define func ptr region
//pub var code: std.ArrayList(dstr) = undefined; // define func region
pub var code: std.ArrayList(Instruction) = .empty; // was std.ArrayList(dstr)

// types
pub const word = if (build.native) usize else u32;
pub const sword = if (build.native) isize else i32;
pub const float = if (build.native) f64 else f32;

pub const hword = if (build.native)
    if (@bitSizeOf(usize) == 64) u32
    else u16
else
    u16;

pub const shword = if (build.native)
    if (@bitSizeOf(isize) == 64) i32
    else i16
else
    i16;

pub const quword = if (build.native)
    if (@bitSizeOf(usize) == 64) u16
    else u8
else
    u8;

pub const squword = if (build.native)
    if (@bitSizeOf(isize) == 64) i16
    else i8
else
    i8;

pub const dword = if (build.double) u64;
pub const sdword = if (build.double) i64;
pub const dpf = if (build.double) f64;

pub const fword = u32;
pub const sfword = i32;
pub const hfword = u16;
pub const shfword = i16;

pub const spf = f32;

pub const byte = u8;
pub const sbyte = i8;

pub const str = []const u8;
pub const wstr = []const []const u8;
pub const dstr = [][]const u8;

pub const mstr = []u8;
pub const wmstr = []const []u8;
pub const dmstr = [][]u8;

// runtime value type.
pub const Type = enum {
    word,
    sword,
    float,
    str,
};

pub const Value = union(Type) {
    word: word,
    sword: sword,
    float: float,
    str: str,

    pub fn typeOf(self: Value) Type { return self; }

    pub fn asString( self: Value, allocator: std.mem.Allocator) !str {
        return switch (self) {
            .word => |v| std.fmt.allocPrint(allocator, "{d}", .{v}),
            .sword => |v| std.fmt.allocPrint(allocator, "{d}", .{v}),
            .float => |v|  std.fmt.allocPrint(allocator, "{d}", .{v}),
            .str => |v| v,
        };
    }
};

pub const Opcode = enum { New, Escape, Func, Call, If, Return, End };
pub const AddrMode = enum { literal, true_literal, forced_eval, pointer, bare };

pub const Op = enum {
    concat, // string concat
    str_eq, // string equals?
    str_starts, // string starts with?
    str_ends, // string ends with?
    str_contains, // string contains?
    add, // add
    sub, // subtract
    mul, // multiply
    div, // divide
    num_eq, // numer equals?
    num_ne, // number does not equal?
    gt, // number greater than?
    lt, // number less than?
};

pub const ExprOperand = union(enum) {
    literal: Value,
    variable: str,
    op: Op,
};

pub const Instruction = struct {
    op: Opcode,
    dest_text: str,
    dest_mode: AddrMode,
    data_text: str,
    data_mode: AddrMode,
    has_data: bool,
    dest_expr: ?[]const ExprOperand, // set when dest_mode == .true_literal and precompilable
    data_expr: ?[]const ExprOperand, // set when data_mode == .literal and precompilable
};

pub fn persistStr(s: str) !str { // helper for saving strings
    return try mem.persistent().dupe(byte, s);
}

pub fn parseValue(value: str) Value { // helper to guess a type
    if (std.fmt.parseInt(sword, value, 10)) |v| {
        if (std.mem.startsWith(byte, value, "-")) {
            return .{ .sword = v };
        }
    } else |_| {}
    if (std.fmt.parseUnsigned(word, value, 10)) |v| {
        return .{ .word = v };
    } else |_| {}
    if (std.fmt.parseFloat(float, value)) |v| {
        return .{ .float = v };
    } else |_| {}
    return .{ .str = value };
}

pub fn resolveValue(value: str) Value { // helper to resolve a variable
    return data.get(value) orelse parseValue(value);
}

pub fn valueToString(allocator: std.mem.Allocator, value: Value) !str { // helper to convert to a string
    return switch (value) {
        .word => |v| std.fmt.allocPrint(allocator, "{d}", .{v}), // word => string
        .sword => |v| std.fmt.allocPrint(allocator, "{d}", .{v}), // signed word => string
        .float => |v| std.fmt.allocPrint(allocator, "{d}", .{v}), // floating point => string
        .str => |v| v, // string => return
    };
}

pub fn persistValue(value: Value) !Value {
    return switch (value) {
        .str => |s| .{ .str = try persistStr(s) },
        else => value,
    };
}
