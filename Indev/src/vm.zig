const std = @import("std");
const state = @import("state");
const escapes = @import("escapes");
const build = @import("build_options");
const tests = @import("tests");
const docs = @import("docs");
const mem = @import("allocator");
const limits = @import("limits");
const version = @import("version");
const psh = @import("psh");
const platform = @import("platform");
const print = platform.print;
const exit = platform.exit;

var recording: bool = false;

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
    concat, str_eq, str_starts, str_ends, str_contains,
    add, sub, mul, div,
    num_eq, num_ne, gt, lt,
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

// Highest precedence first. Each tier is fully collapsed before moving
// to the next tier that ordering is the entire precedence mechanism.
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

pub fn main() !void { // the VM
    // init allocator
    mem.init();
    defer mem.deinit();
    const allocator = mem.alloc();

    // init tables
    state.data = std.StringHashMap(str).init(allocator);
    state.code = .empty;
    state.codeTable = std.StringHashMap(word).init(allocator);

    defer state.data.deinit();
    defer state.code.deinit(allocator);
    defer state.codeTable.deinit();

    // init stack
    callStack = .empty;

    // argument stuff
//    const Args = try std.process.argsWithAllocator(allocator); // for windows compat
    const Args = try getArgs(allocator); // for windows compat
//    const Args = try std.process.argsAlloc(allocator); // old
//    defer std.process.argsFree(std.heap.page_allocator, Args);

    

    const ArgsNum = try std.fmt.allocPrint(allocator, "{d}", .{Args.len});

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

    if (first_arg) |arg| { // handle docs
        if (std.mem.eql(byte, arg, "docs")) {
            print("{s}", .{docs.read()});
            exit(0);
        }
    }
    
    if (first_arg) |arg| { // handle shell
        if (std.mem.eql(byte, arg, "psh")) {
            try run(psh.psh);
            exit(0);
        }
    }


    const filename = first_arg orelse { return; };
    const fileData = try readFile(filename);
    try run(fileData);
}

pub fn run(fileData: str) !void {
    const allocator = mem.alloc();
    var lines = std.mem.splitScalar(byte, fileData, '\n');

    while (lines.next()) |line| {
        if (line.len == 0) continue;
        const IR = try tokenize(line);
        if (IR.len == 0) continue; // skip newlines and comments and such
        defer allocator.free(IR);
        
        if (build.VMDEBUG) {
            try debugDump(IR); // debug dump
            platform.print("Press the Enter key to run said instruction.", .{});
            _ = platform.input();
            platform.print("Running said instruction.", .{});
            try interpret(IR);
        } else {
            try interpret(IR);
        }
    }

//    defer allocator.free(fileData);
}

// read a file
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
        while (i < line.len and std.ascii.isWhitespace(line[i])) { // skip whitespace
            i += 1;
        }

        if (i >= line.len) break;

        if (line[i] == '#') { // comment
            i += 1;
            break;
        }
 
        if (line[i] == '/') { // comment
            i += 1;
            break;
        }

        if (line[i] == '<') { // pointer
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '>') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "/////"); // append "/////" if it was a pointer

            i += 1; // skip closing quote
            continue;
        }

        if (line[i] == '"') { // quoted string
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '"') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "//"); // append "//" if it was quoted

            i += 1; // skip closing quote
            continue;
        }

        if (line[i] == '{') { // force eval
            i += 1;
            const start = i;

            while (i < line.len and line[i] != '}') {
                i += 1;
            }

            if (i >= line.len)
                return error.UnterminatedString;

            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "////"); // append "////" if it was in brackets

            i += 1; // skip closing quote
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
            try tokens.append(allocator, "///"); // append "///" if it was a true literal

            i += 1; // skip closing quote
            continue;
        }

        // Normal token
        const start = i;
        while (i < line.len and
               !std.ascii.isWhitespace(line[i])) {
            i += 1;
        }
        try tokens.append(allocator, line[start..i]);
        try tokens.append(allocator, "/"); // append "/" if it was unquoted
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

fn interpret(list: dstr) !void {
    const allocator = mem.alloc();
    
    if (limits.inst_curr > limits.inst_max) {
        print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
        exit(1);
    }

    limits.inst_curr += 1;

    if (recording) {
        if (std.mem.eql(byte, list[0], "End")) {
            try state.code.append(allocator, try copyInstruction(list));
            recording = false;
            return;
        } else {
            try state.code.append(allocator, try copyInstruction(list));
            return;
        }
    }

    if (std.mem.eql(byte, list[0], "New")) { // New
        if (limits.inst_new_curr > limits.inst_new_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_new_curr += 1;
        var dest: str = undefined;

        // arguments shall come immediatly after the data payloads
        // for this, it shall be
        // 0 = instr, 1 = instraddr, 2 = dest, 3 = destaddr, 4 = data, 5 = dataaddr

        // first argument
        if (std.mem.eql(byte, list[3], "///")) { // literal
            dest = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            dest = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            dest = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            dest = indirect;
        } else {
            return error.UnknownAddressingMode;
        } // does not support copy


        // second argument
        if (std.mem.eql(byte, list[5], "//")) { // literal
            try state.data.put(dest, try evaluate(list[4]));
        } else if (std.mem.eql(byte, list[5], "////")) { // forced eval
            const indirect = state.data.get(list[4]) orelse return error.UnknownVariable;
            const data = try evaluate(indirect);
            try state.data.put(dest, data);
        } else if (std.mem.eql(byte, list[5], "///")) { // true literal
            try state.data.put(dest, list[4]);
        } else if (std.mem.eql(byte, list[5], "/")) { // copy
            const indirect = state.data.get(list[4]) orelse return error.UnknownVariable;
            try state.data.put(dest, indirect);
        } else if (std.mem.eql(byte, list[5], "/////")) { // pointer
            const indirect = state.data.get(list[4]) orelse return error.UnknownVariable;
            try state.data.put(dest, list[4]); // just place the name of the variable in
        } else {
            return error.UnknownAddressingMode;
        }
    }

    if (std.mem.eql(byte, list[0], "Escape")) { // Escape
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
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            escapename = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            escapename = state.data.get(list[2]) orelse return error.UnknownVariable;
        } else { // do not support copy
            return error.InvalidAddressingMode;
        }

        const escape = findEscape(escapename) orelse return error.UnknownEscape;
        try escape.run();
    }

    if (std.mem.eql(byte, list[0], "Func")) { // Start recording functions
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_func_curr += 1;
        recording = true;
        var funcname: str = undefined;

        if (std.mem.eql(byte, list[3], "//")) { // literal
            funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////"))  { // forced eval
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = indirect;
        } else { // do not support copy
            return error.InvalidAddressingMode;
        }

        try state.codeTable.put(funcname, @intCast(state.code.items.len + 1));
        try state.code.append(allocator, try copyInstruction(list));
    }

    if (std.mem.eql(byte, list[0], "Call")) {
        var funcname: str = undefined;

        if (std.mem.eql(byte, list[3], "//")) { // literal
            funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/")) { // true literal
            funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = indirect;
        } else { // do not support copy
            return error.UnknownAddressingMode;
        } 

        const start = state.codeTable.get(funcname)
            orelse return error.UnknownFunction;

        try callStack.append(allocator, .{
            .pc = start,
        });

        return try callFunc();
    }  
 
    if (std.mem.eql(byte, list[0], "If")) { 
        // Start doing Ifs (If funcToExec "condition")
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_if_curr += 1;
        var result: bool = false;
        var funcname: str = undefined;

        // first operand
        if (std.mem.eql(byte, list[3], "/")) { // literal
            funcname = list[2];
        } else if (std.mem.eql(byte, list[3], "///")) { // true literal
            funcname = try evaluate(list[2]);
        } else if (std.mem.eql(byte, list[3], "////")) { // forced eval
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = try evaluate(indirect);
        } else if (std.mem.eql(byte, list[3], "/////")) { // pointer
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            funcname = indirect;
        } else { // do not support copy
            return error.UnknownAddressingMode;
        }

        // second operand
        if (std.mem.eql(byte, list[5], "//")) { // literal
            if (std.mem.eql(byte, try evaluate(list[4]), "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "///")) { // true literal
            if (std.mem.eql(byte, list[4], "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "////")) { // forced eval
            const indirect = state.data.get(list[4]) orelse return error.UnknownVariable;
            if (std.mem.eql(byte, try evaluate(indirect), "0"))
                result = true;
        } else if (std.mem.eql(byte, list[5], "/////")) { // pointer
            const indirect = state.data.get(list[4]) orelse return error.UnknownVariable;
            if (std.mem.eql(byte, indirect, "0"))
                result = true;
        } else { // do not support copy
            return error.UnknownAddressingMode;
        }


        const start = state.codeTable.get(funcname)
            orelse return error.UnknownFunction;

        // new code actually doing shit correctly
        try callStack.append(allocator, .{
            .pc = start,
        });

        if (result)
            return try callFunc();

        // old code that used to jump
//        callStack.items[callStack.items.len - 1].pc = start - 1;    
//        return;
    }

    if (std.mem.eql(byte, list[0], "Return")) { // thing to exit from Func
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

        const instruction = state.code.items[callStack.items[current].pc];

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
        if (std.mem.eql(byte, t, tok)) return true;
    }
    return false;
}

fn resolveOne(tok: str) str {
    return state.data.get(tok) orelse tok;
}

fn applyOp(op: Op, left: str, right: str) EvalError!str {
    const allocator = mem.alloc();
    return switch (op) {
        .concat => try std.fmt.allocPrint(allocator, "{s}{s}", .{ left, right }),
        .str_eq => if (std.mem.eql(byte, left, right)) "0" else "1",
        .str_starts => if (std.mem.startsWith(byte, left, right)) "0" else "1",
        .str_ends => if (std.mem.endsWith(byte, left, right)) "0" else "1",
        .str_contains => if (std.mem.indexOf(byte, left, right) != null) "0" else "1",
        else => {
            const left_num = std.fmt.parseFloat(float, left) catch return error.NotNumeric;
            const right_num = std.fmt.parseFloat(float, right) catch return error.NotNumeric;
            return switch (op) {
                .add => try std.fmt.allocPrint(allocator, "{d}", .{left_num + right_num}),
                .sub => try std.fmt.allocPrint(allocator, "{d}", .{left_num - right_num}),
                .mul => try std.fmt.allocPrint(allocator, "{d}", .{left_num * right_num}),
                .div => try std.fmt.allocPrint(allocator, "{d}", .{left_num / right_num}),
                .num_eq => if (left_num == right_num) "0" else "1",
                .num_ne => if (left_num != right_num) "0" else "1",
                .gt => if (left_num > right_num) "0" else "1",
                .lt => if (left_num < right_num) "0" else "1",
                else => unreachable,
            };
        },
    };
}

// One left-to-right pass over `tokens`, collapsing only the operators
// in `tier`. Anything not in this tier is copied straight through —
// a later pass will see it.
fn reducePass(
    tier: wstr,
    tokens: wstr,
) EvalError!wstr {
    const allocator = mem.alloc();
    var out: std.ArrayList(str) = .empty;
    var i: word = 0;
    while (i < tokens.len) {
        const is_reducible_op = i > 0 and i + 1 < tokens.len and
            isOperator(tokens[i]) and inTier(tier, tokens[i]);
        if (is_reducible_op) {
            const left = resolveOne(out.pop().?);
            const right = resolveOne(tokens[i + 1]);
            const op = op_kind.get(tokens[i]).?;
            const result = try applyOp(op, left, right);
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
    while (it.next()) |t| try token_list.append(allocator, t);

    if (token_list.items.len < 3 or !isOperator(token_list.items[1])) {
        return resolveVariables(line);
    }

    var current: wstr = token_list.items;
    for (tiers) |tier| {
        current = reducePass(tier, current) catch {
            return resolveVariables(line);
        };
    }

    if (current.len != 1) return resolveVariables(line);
    return allocator.dupe(byte, current[0]);
}
