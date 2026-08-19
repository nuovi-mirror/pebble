const std = @import("std");
const state = @import("state");
const escapes = @import("escapes");
const build = @import("build_options");
const tests = @import("tests");
const docs = @import("docs");
const mem = @import("allocator");
const limits = @import("limits");
const version = @import("version");
const psh = @embedFile("psh");
const platform = @import("platform");
const print = platform.print;
const exit = platform.exit;

var recording: bool = false;
var recording_depth: word = 0;

var first_arg: ?str = null;
var second_arg: ?str = null;

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

// evaluator related
const EvalError = error{NotNumeric} || std.mem.Allocator.Error;

pub const Op = enum {
    concat,
    str_eq,
    str_starts,
    str_ends,
    str_contains,
    add,
    sub,
    mul,
    div,
    num_eq,
    num_ne,
    gt,
    lt,
};

const op_kind = std.StaticStringMap(Op).initComptime(.{
    .{ "s++", .concat },
    .{ "?=", .str_eq },
    .{ "s?=", .str_starts },
    .{ "e?=", .str_ends },
    .{ "-?=", .str_contains },
    .{ "+", .add },
    .{ "-", .sub },
    .{ "*", .mul },
    .{ "/", .div },
    .{ "==", .num_eq },
    .{ "!=", .num_ne },
    .{ ">", .gt },
    .{ "<", .lt },
});

const tiers = [_]wstr{
    &.{ "*", "/" },
    &.{ "+", "-", "s++" },
    &.{ "?=", "s?=", "e?=", "-?=", "==", "!=", ">", "<" },
};

// stack-related
const Frame = struct {
    pc: word,
};

var callStack: std.ArrayList(Frame) = .empty;
var funcname: str = undefined;

// for windows compat
fn getArgs(allocator: std.mem.Allocator) !dstr {
    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    var list: std.ArrayList(str) = .empty;

    while (args.next()) |arg| {
        try list.append(allocator, arg);
    }

    return try list.toOwnedSlice(allocator);
}

pub fn main() !void {
    mem.init();
    defer mem.deinit();
    const allocator = mem.alloc();

    state.data = std.StringHashMap(str).init(allocator);
    state.code = .empty;
    state.codeTable = std.StringHashMap(word).init(allocator);

    defer state.data.deinit();
    defer state.code.deinit(allocator);
    defer state.codeTable.deinit();

    callStack = .empty;

    const Args = try getArgs(allocator);

    const ArgsNum = try std.fmt.allocPrint(
        allocator,
        "{d}",
        .{Args.len},
    );

    try state.data.put("VMARGC", ArgsNum);

    for (Args, 0..) |Arg, i| {
        const name = try std.fmt.allocPrint(
            allocator,
            "VMARG{d}",
            .{i},
        );

        try state.data.put(name, Arg);
    }

    // compatability shim
    if (Args.len > 1)
        first_arg = Args[1];

    if (Args.len > 2)
        second_arg = Args[2];

    if (first_arg) |arg| { // handle version thing
        if (std.mem.eql(byte, arg, "version")) {
            print("{s}\n", .{version.full});
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle tests
        if (std.mem.eql(byte, arg, "test")) {
            try run(tests.tests);
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle dancing man
        if (std.mem.eql(byte, arg, "dance")) {
            try run(tests.dance);
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle docs
        if (std.mem.eql(byte, arg, "docs")) {
            print("{s}", .{docs.read()});
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle pSH
        if (std.mem.eql(byte, arg, "psh")) {
            try run(psh);
            exit(0);
        }
    }

    const filename = first_arg orelse {
        return;
    };

    const fileData = try readFile(filename);
    try run(fileData);
}

pub fn run(fileData: str) !void {
    const allocator = mem.alloc();
    var lines = std.mem.splitScalar(byte, fileData, '\n');

    while (lines.next()) |line| {
        if (line.len == 0) continue;

        const IR = try tokenize(line);

        if (IR.len == 0) // skip newlines and comments and such
            continue;

        defer allocator.free(IR);

        if (build.debug) { // debug dump
            try debugDump(IR);
            platform.print(
                "Press the Enter key to run said instruction.",
                .{},
            );
            _ = platform.input();
            platform.print("Running said instruction.", .{});
            try interpret(IR);
        } else {
            try interpret(IR);
        }
    }
}

fn readFile(fileName: str) !mstr {
    const allocator = mem.alloc();

    return try std.fs.cwd().readFileAlloc(
        allocator,
        fileName,
        512 * 1024 * 1024,
    );
}

fn tokenize(line: str) !dstr {
    const allocator = mem.alloc();

    var tokens: std.ArrayList(str) = .empty;
    errdefer tokens.deinit(allocator);

    var i: word = 0;

    while (i < line.len) {
        while (i < line.len and std.ascii.isWhitespace(line[i])) {
            i += 1;
        }

        if (i >= line.len)
            break;

        if (line[i] == '#') { // comment (legacy)
            i += 1;
            break;
        }

        if (line[i] == '/') { // comment (modern)
            i += 1;
            break;
        }

        if (line[i] == '<') {
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '>') { // pointer
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "/////");

            i += 1;
            continue;
        }

        if (line[i] == '"') { // literal
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '"') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "//");

            i += 1;
            continue;
        }

        if (line[i] == '{') { // forced eval
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '}') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "////");

            i += 1;
            continue;
        }

        if (line[i] == '\'') { // true literal
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '\'') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "///");

            i += 1;
            continue;
        }

        const start = i;

        while (i < line.len and
            !std.ascii.isWhitespace(line[i]))
        {
            i += 1;
        }

        try tokens.append(allocator, line[start..i]);
        try tokens.append(allocator, "/");
    }

    return try tokens.toOwnedSlice(allocator);
}

fn findEscape(name: str) ?escapes.Escape {
    for (escapes.table) |escape| {
        if (std.mem.eql(byte, escape.name, name)) {
            return escape;
        }
    }

    return null;
}

// helper for recursive function handling
fn findFuncEnd(start: word) !word {
    var depth: word = 1;
    var pc = start + 1;

    while (pc < state.code.items.len) : (pc += 1) {
        const instruction = state.code.items[pc];

        if (std.mem.eql(byte, instruction[0], "Func")) {
            depth += 1;
        } else if (std.mem.eql(byte, instruction[0], "End")) {
            depth -= 1;

            if (depth == 0)
                return pc;
        }
    }

    return error.InvalidFunction;
}

fn interpret(list: dstr) !void {
    const allocator = mem.alloc();

    if (limits.inst_curr > limits.inst_max) {
        print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
        exit(1);
    }

    limits.inst_curr += 1;

    // arguments shall come immediatly after the data payloads
    // for this, it shall be
    // 0 = instr, 1 = instraddr, 2 = dest, 3 = destaddr, 4 = data, 5 = dataaddr

    // func recording only used when loading source code
    if (recording) {
        if (std.mem.eql(byte, list[0], "Func")) {
            recording_depth += 1;

            try state.code.append(
                allocator,
                try copyInstruction(list),
            );

            return;
        }

        if (std.mem.eql(byte, list[0], "End")) {
            try state.code.append(
                allocator,
                try copyInstruction(list),
            );

            if (recording_depth == 0) {
                recording = false;
            } else {
                recording_depth -= 1;
            }

            return;
        }

        try state.code.append(
            allocator,
            try copyInstruction(list),
        );

        return;
    }

    if (std.mem.eql(byte, list[0], "New")) {
        if (limits.inst_new_curr > limits.inst_new_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_new_curr += 1;

        var dest: str = undefined;

        // first arg
        if (std.mem.eql(byte, list[3], "///")) { // literal
            dest = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect =
                state.data.get(list[2]) orelse
                return error.UnknownVariable;

            dest = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            dest = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect =
                state.data.get(list[2]) orelse
                return error.InvalidPointer;

            dest = indirect;
        } else { // does not support copy
            return error.UnknownAddressingMode;
        }

        // second arg
        if (std.mem.eql(byte, list[5], "//")) { // literal
            try state.data.put(dest, try evaluate(list[4]));
        } else if (std.mem.eql(byte, list[5], "////")) { // forced eval
            const indirect =
                state.data.get(list[4]) orelse
                return error.UnknownVariable;

            const data = try evaluate(indirect);
            try state.data.put(dest, data);
        } else if (std.mem.eql(byte, list[5], "///")) { // true literal
            try state.data.put(dest, list[4]);
        } else if (std.mem.eql(byte, list[5], "/")) { // copy
            const indirect =
                state.data.get(list[4]) orelse
                return error.UnknownVariable;

            try state.data.put(dest, indirect);
        } else if (std.mem.eql(byte, list[5], "/////")) { // pointer
            _ = state.data.get(list[4]) orelse
                return error.InvalidPointer;

            try state.data.put(dest, list[4]);
        } else {
            return error.UnknownAddressingMode;
        }
    }

    if (std.mem.eql(byte, list[0], "Escape")) {
        if (limits.inst_escape_curr > limits.inst_escape_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_escape_curr += 1;

        var escapename: str = undefined;

        if (std.mem.eql(byte, list[3], "/")) { // true literal
            escapename = list[2];
        } else if (std.mem.eql(byte, list[3], "///")) { // literal
            escapename = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect =
                state.data.get(list[2]) orelse
                return error.UnknownVariable;

            escapename = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            escapename =
                state.data.get(list[2]) orelse
                return error.InvalidPointer;
        } else {
            return error.InvalidAddressingMode;
        }

        const escape =
            findEscape(escapename) orelse
            return error.UnknownEscape;

        try escape.run();
    }

    if (std.mem.eql(byte, list[0], "Func")) {
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_func_curr += 1;

//        var funcname: str = undefined;

        if (std.mem.eql(byte, list[3], "//")) { // literal
            funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect =
                state.data.get(list[2]) orelse
                return error.UnknownVariable;

            funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect =
                state.data.get(list[2]) orelse
                return error.InvalidPointer;

            funcname = indirect;
        } else {
            return error.InvalidAddressingMode;
        }

        // runtime-defined func (recursive func)
        if (callStack.items.len > 0) {
            const pc =
                callStack.items[callStack.items.len - 1].pc;

            const end = try findFuncEnd(pc);

            try state.codeTable.put(funcname, pc + 1);

            callStack.items[callStack.items.len - 1].pc = end;

            return;
        }

        // normal func
        recording = true;
        recording_depth = 0;

        try state.codeTable.put(
            funcname,
            @intCast(state.code.items.len + 1),
        );

        try state.code.append(
            allocator,
            try copyInstruction(list),
        );
    }

    if (std.mem.eql(byte, list[0], "Call")) {
        var curr_funcname: str = undefined;

        if (std.mem.eql(byte, list[3], "//")) { // literal
            curr_funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect =
                state.data.get(list[2]) orelse
                return error.UnknownVariable;

            curr_funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            curr_funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect =
                state.data.get(list[2]) orelse
                return error.InvalidPointer;

            curr_funcname = indirect;
        } else {
            return error.UnknownAddressingMode;
        }

        const start =
            state.codeTable.get(curr_funcname) orelse
            return error.UnknownFunction;

        if (std.mem.eql(byte, funcname, curr_funcname)) {
            if (recording) {
                // jump
                callStack.items[callStack.items.len - 1].pc = start;
                return;
            } else {
                // call function
                try callStack.append(allocator, .{
                    .pc = start,
                });

                return try callFunc();
            }
        } else {
            // call function
            try callStack.append(allocator, .{
                .pc = start,
            });

            return try callFunc();
        }
    }

    if (std.mem.eql(byte, list[0], "If")) {
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_if_curr += 1;

        var result: bool = false;
//        var funcname: str = undefined;
        var curr_funcname: str = undefined;

        if (std.mem.eql(byte, list[3], "/")) { // true literal
            curr_funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "///")) { // literal
            curr_funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect =
                state.data.get(list[2]) orelse
                return error.UnknownVariable;

            curr_funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect =
                state.data.get(list[2]) orelse
                return error.InvaidPointer;

            curr_funcname = indirect;
        } else {
            return error.UnknownAddressingMode;
        }

        if (std.mem.eql(byte, list[5], "//")) { // literal
            if (std.mem.eql(byte, try evaluate(list[4]), "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "///")) { // true literal
            if (std.mem.eql(byte, list[4], "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "////")) { // forced eval
            const indirect =
                state.data.get(list[4]) orelse
                return error.UnknownVariable;

            if (std.mem.eql(byte, try evaluate(indirect), "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "/////")) { // pointer
            const indirect =
                state.data.get(list[4]) orelse
                return error.InvalidPointer;

            if (std.mem.eql(byte, indirect, "0"))
                result = true;
        } else {
            return error.UnknownAddressingMode;
        }

        const start =
            state.codeTable.get(funcname) orelse
            return error.UnknownFunction;

        if (result) {
            if (std.mem.eql(byte, funcname, curr_funcname)) {
                if (recording) {
                    // jump
                    callStack.items[callStack.items.len - 1].pc = start;
                    return;
                } else {
                    // call function
                    try callStack.append(allocator, .{
                        .pc = start,
                    });

                    return try callFunc();
                }
            } else {
                // call function
                try callStack.append(allocator, .{
                    .pc = start,
                });

                return try callFunc();
            }
        }
    }

    if (std.mem.eql(byte, list[0], "Return")) {
        if (limits.inst_return_curr > limits.inst_return_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_return_curr += 1;

        return error.Return;
    }
}

fn callFunc() anyerror!void {
    const base = callStack.items.len - 1;

    while (callStack.items.len > base) {
        const current = callStack.items.len - 1;

        if (callStack.items[current].pc >= state.code.items.len) {
            return error.InvalidPC;
        }

        const instruction =
            state.code.items[callStack.items[current].pc];

        if (std.mem.eql(byte, instruction[0], "End")) {
            _ = callStack.pop();
            continue;
        }

        interpret(instruction) catch |err| {
            if (err == error.Return) {
                _ = callStack.pop();
                continue;
            }

            return err;
        };

        if (callStack.items.len > base) {
            callStack.items[callStack.items.len - 1].pc += 1;
        }
    }
}

fn isOperator(op: str) bool {
    return std.mem.eql(byte, op, "s++") or
        std.mem.eql(byte, op, "?=") or
        std.mem.eql(byte, op, "s?=") or
        std.mem.eql(byte, op, "e?=") or
        std.mem.eql(byte, op, "-?=") or
        std.mem.eql(byte, op, "+") or
        std.mem.eql(byte, op, "-") or
        std.mem.eql(byte, op, "*") or
        std.mem.eql(byte, op, "/") or
        std.mem.eql(byte, op, "==") or
        std.mem.eql(byte, op, "!=") or
        std.mem.eql(byte, op, ">") or
        std.mem.eql(byte, op, "<");
}

fn resolveVariables(line: str) !str {
    const allocator = mem.alloc();

    var output: std.ArrayList(byte) = .empty;
    defer output.deinit(allocator);

    var parts = std.mem.splitScalar(byte, line, ' ');
    var first = true;

    while (parts.next()) |part| {
        if (!first) {
            try output.append(allocator, ' ');
        }

        first = false;

        if (state.data.get(part)) |data| {
            try output.appendSlice(allocator, data);
        } else {
            try output.appendSlice(allocator, part);
        }
    }

    return try output.toOwnedSlice(allocator);
}

fn copyInstruction(list: dstr) !dstr {
    const allocator = mem.alloc();

    var copy = try allocator.alloc(str, list.len);

    for (list, 0..) |token, i| {
        copy[i] = try allocator.dupe(byte, token);
    }

    return copy;
}

fn debugDump(ir: dstr) !void {
    const capacity = mem.queryCapacity();
    const allocator = mem.alloc();

    print("\n======== VM DEBUG ========\n", .{});

    print("Instruction: ", .{});

    for (ir) |part| {
        print("{s} ", .{part});
    }

    print("\n", .{});

    print("CODE:\n", .{});

    for (state.code.items) |item| {
        print("  ", .{});

        for (item) |token| {
            print("{s} ", .{token});
        }

        print("\n", .{});
    }

    print("Recording: {}\n", .{recording});
    print("Recording depth: {}\n", .{recording_depth});

    print("CODE TABLE:\n", .{});

    var code_iter = state.codeTable.iterator();

    while (code_iter.next()) |entry| {
        print("  {s} = {}\n", .{
            entry.key_ptr.*,
            entry.value_ptr.*,
        });
    }

    print("DATA:\n", .{});

    var data_iter = state.data.iterator();

    while (data_iter.next()) |entry| {
        print("  {s} = {s}\n", .{
            entry.key_ptr.*,
            entry.value_ptr.*,
        });
    }

    print("ESCAPES:\n", .{});
    escapes.dump();

    const cstring = try std.fmt.allocPrint(
        allocator,
        "{}",
        .{capacity / 1024 / 1024},
    );

    print("MEM: {s}MB\n", .{cstring});
    print("STACK: {}\n", .{callStack.items.len});

    print("\n======== VM DEBUG ========\n", .{});
}

fn inTier(tier: wstr, tok: str) bool {
    for (tier) |t| {
        if (std.mem.eql(byte, t, tok))
            return true;
    }

    return false;
}

fn resolveOne(tok: str) str {
    return state.data.get(tok) orelse tok;
}

fn applyOp(
    op: Op,
    left: str,
    right: str,
) EvalError!str {
    const allocator = mem.alloc();

    return switch (op) {
        .concat =>
            try std.fmt.allocPrint(
                allocator,
                "{s}{s}",
                .{ left, right },
            ),

        .str_eq =>
            if (std.mem.eql(byte, left, right))
                "0"
            else
                "1",

        .str_starts =>
            if (std.mem.startsWith(byte, left, right))
                "0"
            else
                "1",

        .str_ends =>
            if (std.mem.endsWith(byte, left, right))
                "0"
            else
                "1",

        .str_contains =>
            if (std.mem.indexOf(byte, left, right) != null)
                "0"
            else
                "1",

        else => {
            const left_num =
                std.fmt.parseFloat(float, left)
                catch return error.NotNumeric;

            const right_num =
                std.fmt.parseFloat(float, right)
                catch return error.NotNumeric;

            return switch (op) {
                .add =>
                    try std.fmt.allocPrint(
                        allocator,
                        "{d}",
                        .{left_num + right_num},
                    ),

                .sub =>
                    try std.fmt.allocPrint(
                        allocator,
                        "{d}",
                        .{left_num - right_num},
                    ),

                .mul =>
                    try std.fmt.allocPrint(
                        allocator,
                        "{d}",
                        .{left_num * right_num},
                    ),

                .div =>
                    try std.fmt.allocPrint(
                        allocator,
                        "{d}",
                        .{left_num / right_num},
                    ),

                .num_eq =>
                    if (left_num == right_num)
                        "0"
                    else
                        "1",

                .num_ne =>
                    if (left_num != right_num)
                        "0"
                    else
                        "1",

                .gt =>
                    if (left_num > right_num)
                        "0"
                    else
                        "1",

                .lt =>
                    if (left_num < right_num)
                        "0"
                    else
                        "1",

                else => unreachable,
            };
        },
    };
}

fn reducePass(
    tier: wstr,
    tokens: wstr,
) EvalError!wstr {
    const allocator = mem.alloc();

    var out: std.ArrayList(str) = .empty;
    var i: word = 0;

    while (i < tokens.len) {
        const is_reducible_op =
            i > 0 and
            i + 1 < tokens.len and
            isOperator(tokens[i]) and
            inTier(tier, tokens[i]);

        if (is_reducible_op) {
            const left = resolveOne(out.pop().?);
            const right = resolveOne(tokens[i + 1]);

            const op = op_kind.get(tokens[i]).?;

            const result =
                try applyOp(op, left, right);

            try out.append(allocator, result);

            i += 2;
        } else {
            try out.append(allocator, tokens[i]);
            i += 1;
        }
    }

    return try out.toOwnedSlice(allocator);
}

pub fn evaluate(line: str) !str {
    const allocator = mem.alloc();

    var token_list: std.ArrayList(str) = .empty;

    var it = std.mem.splitScalar(byte, line, ' ');

    while (it.next()) |t|
        try token_list.append(allocator, t);

    if (token_list.items.len < 3 or
        !isOperator(token_list.items[1]))
    {
        return resolveVariables(line);
    }

    var current: wstr = token_list.items;

    for (tiers) |tier| {
        current = reducePass(tier, current) catch {
            return resolveVariables(line);
        };
    }

    if (current.len != 1)
        return resolveVariables(line);

    return allocator.dupe(byte, current[0]);
}
