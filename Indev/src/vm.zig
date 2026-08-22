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

const word = state.word;
const hword = state.hword;
const byte = state.byte;
const sword = state.sword;
const shword = state.shword;
const spf = state.spf;
const float = state.float;
const str = state.str;
const wstr = state.wstr;
const dstr = state.dstr;
const mstr = state.mstr;
const wmstr = state.wmstr;
const dmstr = state.dmstr;

// legacy compat
const valueToString = state.valueToString;
const persistValue = state.persistValue;
const parseValue = state.parseValue;
const resolveVariable = state.resolveVariable;
const resolveValue = state.resolveValue;
const persistStr = state.persistStr;
// - - - - - - -

const Instruction = state.Instruction;
const Opcode = state.Opcode;
const AddrMode = state.AddrMode;
const ExprOperand = state.ExprOperand;
const Op = state.Op;

// evaluator-related
const EvalError = error{NotNumeric} || std.mem.Allocator.Error;

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

const op_tiers = [_][]const Op{
    &.{ .mul, .div },
    &.{ .add, .sub, .concat },
    &.{ .str_eq, .str_starts, .str_ends, .str_contains, .num_eq, .num_ne, .gt, .lt },
};

const opcode_kind = std.StaticStringMap(Opcode).initComptime(.{ // table of opcodes
    .{ "New", .New },
    .{ "Escape", .Escape },
    .{ "Func", .Func },
    .{ "Call", .Call },
    .{ "If", .If },
    .{ "Return", .Return },
    .{ "End", .End },
});

const addr_mode_kind = std.StaticStringMap(AddrMode).initComptime(.{ // table of addressing modes
    .{ "//", .literal },
    .{ "///", .true_literal },
    .{ "////", .forced_eval },
    .{ "/////", .pointer },
    .{ "/", .bare },
});

const Frame = struct { // stack data
    pc: word, // program counter
    name: str, // function name
};
var callStack: std.ArrayList(Frame) = .empty; // the stack itself

const Operand = union(enum) { // operands for instructions
    token: str,
    value: state.Value,
};

const RtOperand = union(enum) {
    value: state.Value,
    op: Op,
};

fn toScratch(value: state.Value) !state.Value { // helper for scratching data
    return switch (value) {
        .str => |s| .{ .str = try mem.scratch().dupe(byte, s) },
        else => value,
    };
}

fn getArgs(allocator: std.mem.Allocator) !dstr { // helper for argument handling
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

    const allocator = mem.persistent(); // set the default allocator to the persistent allocator

    state.data = std.StringHashMap(state.Value).init(mem.persistent()); // initalize the variable area
    state.codeTable = std.StringHashMap(word).init(mem.persistent()); // initalize the function pointer area
    state.code = .empty; // initalize the function area

    defer state.data.deinit();
    defer state.code.deinit(allocator);
    defer state.codeTable.deinit();

    callStack = .empty; // initalize stack
    recording = false; // initalize recording
    recording_depth = 0; // initalize recording depth

    const Args = try getArgs(allocator);

    try state.data.put("VMARGC", .{ .word = @intCast(Args.len) }); // must be done before starting a program
    for (Args, 0..) |Arg, i| { // must be done before starting a program
        const name = try std.fmt.allocPrint(allocator, "VMARG{d}", .{i});
        try state.data.put(name, .{ .str = Arg });
    }

    if (Args.len > 1) first_arg = Args[1]; // wrapper for legacy compat
    if (Args.len > 2) second_arg = Args[2]; // wrapper for legacy compat

    if (first_arg) |arg| { // handle version metadata fetch
        if (std.mem.eql(byte, arg, "version")) {
            print("{s}\n", .{version.full});
            exit(0);
        }
    }

    if (first_arg) |arg| { // hanle test suite
        if (std.mem.eql(byte, arg, "test")) {
            try run(tests.tests);
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle the dancing man program
        if (std.mem.eql(byte, arg, "dance")) {
            try run(tests.dance);
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle the documentation
        if (std.mem.eql(byte, arg, "docs")) {
            print("{s}", .{docs.read()});
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle the Pebble SHell (pSH)
        if (std.mem.eql(byte, arg, "psh")) {
            try run(psh);
            exit(0);
        }
    }

    // first argument can be assumed to be the file to run
    // containing source textual bytecode after this point

    const filename = first_arg orelse return;
    const fileData = try readFile(filename);
    try run(fileData); // run the program specified
}

pub fn run(fileData: str) !void { // use this to run a full program
    // hand this function a full program formatted as
    // Pebble textual bytecode to start execution
    // this does not handle the driver loop itself
    var lines = std.mem.splitScalar(byte, fileData, '\n');

    while (lines.next()) |line| {
        if (line.len == 0) continue; // skip empty lines

        const IR = try tokenize(line); // attempt to tokenize it
        if (IR.len == 0) continue; // skip if failed

        const instr = try compileInstruction(mem.persistent(), IR); // attempt to compile the tokenized
                                                                    // instruction data
        if (build.debug) { // handle debugger
            try debugDump(instr);
            platform.print("Press the Enter key to run said instruction.", .{});
            _ = platform.input();
            platform.print("Running said instruction.", .{});
        }

        try interpret(instr); // interpret the compiled bytecode
        if (callStack.items.len > 0) try drive(); // run the main driver
    }
}

fn readFile(fileName: str) !mstr { // helper for reading files
    const allocator = mem.persistent(); // set the default allocator to the persistent allocator
    return try std.fs.cwd().readFileAlloc(
        allocator,
        fileName,
        512 * 1024 * 1024,
    );
}

fn tokenize(line: str) !dstr { // helper for tokenizing source textual bytecode
    const allocator = mem.scratch(); // set the default allocator to the scratch allocator
    var tokens: std.ArrayList(str) = .empty;
    var i: word = 0;

    errdefer tokens.deinit(allocator);

    // all this really does is format to a simple IR
    // really just parse addressing modes and separate
    // newliens - formatted as the opcode, followed by
    // a reserve, followed by each operand with its 
    // addressing mode ,eg. [foo, RESERVE, bar, addr, baz, addr]

    while (i < line.len) {
        while (i < line.len and std.ascii.isWhitespace(line[i])) { i += 1; } // skip whitespaces

        if (i >= line.len) break; // skip empty lines
        if (line[i] == '#') { i += 1; break; } // skip legacy comments
        if (line[i] == '/') { i += 1; break; } // skip modern comments

        if (line[i] == '<') { // tokenize pointers
            i += 1;
            const start = i;
            while (i < line.len and line[i] != '>') { i += 1; }
            if (i >= line.len) return error.UnterminatedString;
            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "/////"); // append symbol for token
            i += 1;
            continue;
        }

        if (line[i] == '"') { // tokenize literals
            i += 1;
            const start = i;
            while (i < line.len and line[i] != '"') { i += 1; }
            if (i >= line.len) return error.UnterminatedString;
            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "//"); // append symbol for token
            i += 1;
            continue;
        }

        if (line[i] == '{') { // tokenize forced eval
            i += 1;
            const start = i;
            while (i < line.len and line[i] != '}') { i += 1; }
            if (i >= line.len) return error.UnterminatedString;
            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "////"); // append symbol for token
            i += 1;
            continue;
        }

        if (line[i] == '\'') { // tokenize true literals
            i += 1;
            const start = i;
            while (i < line.len and line[i] != '\'') { i += 1; }
            if (i >= line.len) return error.UnterminatedString;
            try tokens.append(allocator, line[start..i]);
            try tokens.append(allocator, "///"); // append symbol for token
            i += 1;
            continue;
        }

        const start = i;
        while (i < line.len and !std.ascii.isWhitespace(line[i])) { i += 1; }
        try tokens.append(allocator, line[start..i]);
        try tokens.append(allocator, "/"); // append symbol for token
    }

    return try tokens.toOwnedSlice(allocator);
}

fn findEscape(name: str) ?escapes.Escape { // helper for finding an escape sequence
    for (escapes.table) |escape| {
        if (std.mem.eql(byte, escape.name, name)) return escape;
    }
    return null;
}

// expression pre-compiling code

// converts raw operand text into fixed sequences once at comptime
// literals parsed into state.Value, variables keep their name but
// are not re-tokenized each time

// will return null if the text is not well-formatted
// operand/operator, so just pass it to a plain runtime
// evaluate() if this returns null

// expects input in the IR format described previously

fn classifyOperand(tok: str) ExprOperand { // helper for classifying operands
    const v = parseValue(tok);
    return switch (v) {
        .str => .{ .variable = tok }, // not numeric -> must be a variable reference
        else => .{ .literal = v },
    };
}

fn compileExpression(allocator: std.mem.Allocator, text: str) !?[]const ExprOperand {
    var token_list: std.ArrayList(str) = .empty;
    var it = std.mem.splitScalar(byte, text, ' '); // split on spaces
    
    // create an array from the split IR text
    while (it.next()) |tok| { try token_list.append(mem.temp(), tok); }

    defer mem.resetTemp(); // reset the temporary allocator

    if (token_list.items.len == 1) {
        const out = try allocator.alloc(ExprOperand, 1);
        out[0] = classifyOperand(token_list.items[0]);
        return out;
    }

    if (token_list.items.len < 3 or token_list.items.len % 2 == 0) return null;

    const out = try allocator.alloc(ExprOperand, token_list.items.len);
    for (token_list.items, 0..) |tok, idx| {
        if (idx % 2 == 1) {
            const op = op_kind.get(tok) orelse return null;
            out[idx] = .{ .op = op };
        } else {
            out[idx] = classifyOperand(tok);
        }
    }

    return out;
}

// instruction compiler 

// takes a raw token stream and returns the resolved instruction
// items are only allocated once, not once per execution

fn compileInstruction(allocator: std.mem.Allocator, list: dstr) !Instruction {
    const op = opcode_kind.get(list[0]) orelse return error.UnknownInstruction;

    var dest_text: str = "";
    var dest_mode: AddrMode = .bare;
    var dest_expr: ?[]const ExprOperand = null;

    if (list.len > 3) {
        dest_text = try allocator.dupe(byte, list[2]);
        dest_mode = addr_mode_kind.get(list[3]) orelse return error.UnknownAddressingMode;
        if (dest_mode == .true_literal) {
            dest_expr = try compileExpression(allocator, dest_text);
        }
    }

    const has_data = list.len > 5;
    var data_text: str = "";
    var data_mode: AddrMode = .bare;
    var data_expr: ?[]const ExprOperand = null;

    if (has_data) {
        data_text = try allocator.dupe(byte, list[4]);
        data_mode = addr_mode_kind.get(list[5]) orelse return error.UnknownAddressingMode;
        if (data_mode == .literal) {
            data_expr = try compileExpression(allocator, data_text);
        }
    }

    return .{ // return formatted instruction
        .op = op, // opcode
        .dest_text = dest_text, // destination operand
        .dest_mode = dest_mode, // destination addressing mode
        .data_text = data_text, // data operand
        .data_mode = data_mode, // data addressing mode
        .has_data = has_data, // has data operand?
        .dest_expr = dest_expr, // is the destination operand an expression?
        .data_expr = data_expr, // is the data operand an expression?
    };
}

fn isTailCall(pc: word) bool { // helper to check if there is a tail-call
    const next = pc + 1;
    // if the next instruction is before the end of the code table and we
    // are parsing an end instruction, it is safe to jump
    return next < state.code.items.len and state.code.items[next].op == .End;
}

fn findFuncEnd(start: word) !word { // helper to find the end of a function
    var depth: word = 1;
    var pc = start + 1;
    while (pc < state.code.items.len) : (pc += 1) {
        const instruction = state.code.items[pc];
        if (instruction.op == .Func) {
            depth += 1;
        } else if (instruction.op == .End) {
            depth -= 1;
            if (depth == 0) return pc;
        }
    }
    return error.InvalidFunction;
}

fn pushCall(allocator: std.mem.Allocator, name: str, start: word) !void { // safe helper to manage stack
    try callStack.append(allocator, .{ .pc = start, .name = name });
    if (callStack.items.len > limits.call_depth_max) {
        print("VM: FATAL: CALL STACK OVERFLOW\n", .{});
        exit(1);
    }
}

fn interpret(instr: Instruction) !void { // main interpreter
    const allocator = mem.persistent(); // set the default allocator to the persistent allocator
    mem.resetScratch(); // reset the scratch allocator

    if (limits.inst_curr > limits.inst_max) { // error if over the instruction limit
        print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
        exit(1);
    }

    limits.inst_curr += 1;

    if (recording) { // change behavior if the function is recursive
        try state.code.append(allocator, instr);
        if (instr.op == .Func) {
            recording_depth += 1;
        } else if (instr.op == .End) {
            if (recording_depth == 0) {
                recording = false;
            } else {
                recording_depth -= 1;
            }
        }
        return;
    }

    switch (instr.op) {
        .New => {
            if (limits.inst_new_curr > limits.inst_new_max) {
                print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
                exit(1);
            }
    
            limits.inst_new_curr += 1;

            var dest: str = undefined;

            switch (instr.dest_mode) { // resolve the addressing mode of the destination operand
                .true_literal => {
                    const v = if (instr.dest_expr) |expr|
                        try evalCompiledExpr(expr) // try to evaluate the compiled expression
                    else
                        try evaluate(instr.dest_text); // evaluate at runtime if we cannot
                    dest = try persistStr(try valueToString(mem.temp(), v));
                },
                .forced_eval => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.UnknownVariable;
                    const name = try valueToString(mem.temp(), indirect);
                    dest = try persistStr(try valueToString(mem.temp(), try evaluate(name)));
                },
                .bare => dest = instr.dest_text,
                .pointer => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.InvalidPointer;
                    dest = try persistStr(try valueToString(mem.temp(), indirect));
                },
                .literal => return error.UnknownAddressingMode,
            }

            var value: state.Value = undefined;

            if (!instr.has_data) return error.UnknownAddressingMode;

            switch (instr.data_mode) { // resolve the addressing mode of the data operand
                .literal => {
                    const v = if (instr.data_expr) |expr|
                        try evalCompiledExpr(expr)
                    else
                        try evaluate(instr.data_text);
                    value = try persistValue(v);
                },
                .forced_eval => {
                    const indirect = state.data.get(instr.data_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    value = try persistValue(try evaluate(input));
                },
                .true_literal => value = try persistValue(parseValue(instr.data_text)),
                .bare => {
                    const indirect = state.data.get(instr.data_text) orelse return error.UnknownVariable;
                    value = indirect;
                },
                .pointer => {
                    _ = state.data.get(instr.data_text) orelse return error.InvalidPointer;
                    value = .{ .str = instr.data_text };
                },
            }

            try state.data.put(dest, value);
        },

        .Escape => {
            if (limits.inst_escape_curr > limits.inst_escape_max) {
                print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
                exit(1);
            }
            limits.inst_escape_curr += 1;

            var escapename: str = undefined;

            switch (instr.dest_mode) {
                .bare => escapename = instr.dest_text,
                .true_literal => escapename = try valueToString(mem.temp(), try evaluate(instr.dest_text)),
                .forced_eval => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    escapename = try valueToString(mem.temp(), try evaluate(input));
                },
                .pointer => {
                    const value = state.data.get(instr.dest_text) orelse return error.InvalidPointer;
                    escapename = try valueToString(mem.temp(), value);
                },
                .literal => return error.UnknownAddressingMode,
            }

            const escape = findEscape(escapename) orelse return error.UnknownEscape;
            try escape.run();
        },

        .Func => {
            if (limits.inst_func_curr > limits.inst_func_max) {
                print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
                exit(1);
            }
            limits.inst_func_curr += 1;

            var name: str = undefined;

            switch (instr.dest_mode) {
                .literal => name = try persistStr(try valueToString(mem.temp(), try evaluate(instr.dest_text))),
                .forced_eval => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    name = try persistStr(try valueToString(mem.temp(), try evaluate(input)));
                },
                .bare => name = instr.dest_text,
                .pointer => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.InvalidPointer;
                    name = try persistStr(try valueToString(mem.temp(), indirect));
                },
                .true_literal => return error.UnknownAddressingMode,
            }

            if (callStack.items.len > 0) {
                const pc = callStack.items[callStack.items.len - 1].pc;
                const end = try findFuncEnd(pc);
                try state.codeTable.put(name, pc + 1);
                //callStack.items[callStack.items.len - 1].pc = end;
                callStack.items[callStack.items.len - 1].pc = end + 1; // hotpatch
                return;
            }

            const func_pc: word = @intCast(state.code.items.len);
            try state.code.append(allocator, instr);
            try state.codeTable.put(name, func_pc + 1);

            recording = true;
            recording_depth = 0;
            return;
        },

        .Call => {
            var curr_funcname: str = undefined;

            switch (instr.dest_mode) {
                .literal => curr_funcname = 
                    try persistStr(try valueToString(mem.temp(), try evaluate(instr.dest_text))),
                .forced_eval => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    curr_funcname = try persistStr(try valueToString(mem.temp(), try evaluate(input)));
                },
                .bare => curr_funcname = instr.dest_text,
                .pointer => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.InvalidPointer;
                    curr_funcname = try persistStr(try valueToString(mem.temp(), indirect));
                },
                .true_literal => return error.UnknownAddressingMode,
            }

            const start = state.codeTable.get(curr_funcname) orelse return error.UnknownFunction;

            const is_self_call =
                callStack.items.len > 0 and
                std.mem.eql(byte, callStack.items[callStack.items.len - 1].name, curr_funcname);

            if (is_self_call and isTailCall(callStack.items[callStack.items.len - 1].pc)) {
                callStack.items[callStack.items.len - 1].pc = start;
            } else {
                try pushCall(allocator, curr_funcname, start);
            }

            return;
        },

        .If => {
            if (limits.inst_if_curr > limits.inst_if_max) {
                print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
                exit(1);
            }
            limits.inst_if_curr += 1;

            var result: bool = false;
            var curr_funcname: str = undefined;

            switch (instr.dest_mode) {
                .bare => curr_funcname = instr.dest_text,
                .literal => 
                    curr_funcname = try persistStr(try valueToString(mem.temp(), try evaluate(instr.dest_text))),
                .forced_eval => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    curr_funcname = try persistStr(try valueToString(mem.temp(), try evaluate(input)));
                },
                .pointer => {
                    const indirect = state.data.get(instr.dest_text) orelse return error.InvalidPointer;
                    curr_funcname = try persistStr(try valueToString(mem.temp(), indirect));
                },
                .true_literal => return error.UnknownAddressingMode,
            }

            if (!instr.has_data) return error.UnknownAddressingMode;

            switch (instr.data_mode) {
                .literal => {
                    const v = if (instr.data_expr) |expr|
                        try evalCompiledExpr(expr)
                    else
                        try evaluate(instr.data_text);
                    if (std.mem.eql(byte, try valueToString(mem.temp(), v), "0"))
                        result = true;
                },
                .true_literal => {
                    if (std.mem.eql(byte, instr.data_text, "0"))
                        result = true;
                },
                .forced_eval => {
                    const indirect = state.data.get(instr.data_text) orelse return error.UnknownVariable;
                    const input = try valueToString(mem.temp(), indirect);
                    if (std.mem.eql(byte, try valueToString(mem.temp(), try evaluate(input)), "0"))
                        result = true;
                },
                .pointer => {
                    const indirect = state.data.get(instr.data_text) orelse return error.InvalidPointer;
                    const indirect_string = try valueToString(mem.temp(), indirect);
                    if (std.mem.eql(byte, indirect_string, "0"))
                        result = true;
                },
                .bare => return error.UnknownAddressingMode,
            }

            if (!result) return;

            const start = state.codeTable.get(curr_funcname) orelse return error.UnknownFunction;

            const is_self_call =
                callStack.items.len > 0 and
                std.mem.eql(byte, callStack.items[callStack.items.len - 1].name, curr_funcname);

            if (is_self_call and isTailCall(callStack.items[callStack.items.len - 1].pc)) {
                callStack.items[callStack.items.len - 1].pc = start;
            } else {
                try pushCall(allocator, curr_funcname, start);
            }

            return;
        },

        .Return => {
            if (limits.inst_return_curr > limits.inst_return_max) {
                print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
                exit(1);
            }
            limits.inst_return_curr += 1;
            return error.Return;
        },

        .End => {},
    }
}

fn drive() !void { // main driver
    // used to allow trampoline stuff so we do not pollute the host stack

    while (callStack.items.len > 0) {
        const idx = callStack.items.len - 1;
        const pc_before = callStack.items[idx].pc;

        if (pc_before >= state.code.items.len) return error.InvalidPC;

        const instruction = state.code.items[pc_before];

        if (instruction.op == .End) {
            _ = callStack.pop();
            if (callStack.items.len > 0)
                callStack.items[callStack.items.len - 1].pc += 1;
            continue;
        }

        const depth_before = callStack.items.len;

        interpret(instruction) catch |err| {
            if (err == error.Return) {
                _ = callStack.pop();
                if (callStack.items.len > 0)
                    callStack.items[callStack.items.len - 1].pc += 1;
                continue;
            }
            return err;
        };

        if (callStack.items.len == 0) continue;

        if (callStack.items.len == depth_before) {
            const current = callStack.items.len - 1;
            if (callStack.items[current].pc == pc_before) {
                callStack.items[current].pc += 1;
            }
        }
    }
}

// fast path - evaluate a pre-compiled expression
fn inOpTier(tier: []const Op, op: Op) bool {
    for (tier) |t| {
        if (t == op) return true;
    }
    return false;
}

fn reduceRt(tier: []const Op, operands: []const RtOperand) EvalError![]RtOperand {
    const allocator = mem.temp();
    var out: std.ArrayList(RtOperand) = .empty;
    var i: word = 0;

    while (i < operands.len) {
        const is_reducible =
            i > 0 and
            i + 1 < operands.len and
            operands[i] == .op and
            inOpTier(tier, operands[i].op);

        if (is_reducible) {
            const left = out.pop().?.value;
            const right = operands[i + 1].value;
            const result = try applyOp(operands[i].op, left, right);
            try out.append(allocator, .{ .value = result });
            i += 2;
        } else {
            try out.append(allocator, operands[i]);
            i += 1;
        }
    }

    return try out.toOwnedSlice(allocator);
}

// evaluate a pre-compiled expression
// (AKA the fast path)
fn evalCompiledExpr(expr: []const ExprOperand) EvalError!state.Value {
    defer mem.resetTemp();

    if (expr.len == 1) {
        const v = switch (expr[0]) {
            .literal => |val| val,
            .variable => |name| resolveValue(name),
            .op => unreachable, // malformed single-slot expr; not producible by compileExpression
        };
        return toScratch(v);
    }

    const allocator = mem.temp();
    var current: std.ArrayList(RtOperand) = .empty;

    for (expr) |e| {
        switch (e) {
            .literal => |v| try current.append(allocator, .{ .value = v }),
            .variable => |name| try current.append(allocator, .{ .value = resolveValue(name) }),
            .op => |o| try current.append(allocator, .{ .op = o }),
        }
    }

    var slice: []RtOperand = current.items;

    for (op_tiers) |tier| {
        slice = try reduceRt(tier, slice);
    }

    if (slice.len != 1 or slice[0] != .value) return error.NotNumeric;
    return toScratch(slice[0].value);
}

// dynamic evaluator (fallback - evaluates full text at runtime)
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
    const allocator = mem.temp();
    var output: std.ArrayList(byte) = .empty;
    defer output.deinit(allocator);

    var parts = std.mem.splitScalar(byte, line, ' ');
    var first = true;

    while (parts.next()) |part| {
        if (!first) try output.append(allocator, ' ');
        first = false;

        if (state.data.get(part)) |value| {
            const value_string = try valueToString(allocator, value);
            try output.appendSlice(allocator, value_string);
        } else {
            try output.appendSlice(allocator, part);
        }
    }

    return try output.toOwnedSlice(allocator);
}

fn inTier(tier: wstr, tok: str) bool {
    for (tier) |t| {
        if (std.mem.eql(byte, t, tok)) return true;
    }
    return false;
}

fn resolveOperand(operand: Operand) state.Value {
    return switch (operand) {
        .value => |v| v,
        .token => |tok| resolveValue(tok),
    };
}

fn applyOp(op: Op, left: state.Value, right: state.Value) EvalError!state.Value {
    const allocator = mem.temp();

    return switch (op) {
        .concat => {
            const l = try valueToString(allocator, left);
            const r = try valueToString(allocator, right);
            return .{ .str = try std.fmt.allocPrint(allocator, "{s}{s}", .{ l, r }) };
        },

        .str_eq => {
            const l = try valueToString(allocator, left);
            const r = try valueToString(allocator, right);
            return .{ .word = if (std.mem.eql(byte, l, r)) 0 else 1 };
        },

        .str_starts => {
            const l = try valueToString(allocator, left);
            const r = try valueToString(allocator, right);
            return .{ .word = if (std.mem.startsWith(byte, l, r)) 0 else 1 };
        },

        .str_ends => {
            const l = try valueToString(allocator, left);
            const r = try valueToString(allocator, right);
            return .{ .word = if (std.mem.endsWith(byte, l, r)) 0 else 1 };
        },

        .str_contains => {
            const l = try valueToString(allocator, left);
            const r = try valueToString(allocator, right);
            return .{ .word = if (std.mem.indexOf(byte, l, r) != null) 0 else 1 };
        },

        else => {
            const l = switch (left) {
                .word => |v| @as(float, @floatFromInt(v)),
                .sword => |v| @as(float, @floatFromInt(v)),
                .float => |v| v,
                .str => |v| blk: {
                    if (std.fmt.parseFloat(float, v)) |parsed| {
                        break :blk parsed;
                    } else |_| {
                        return error.NotNumeric;
                    }
                },
            };

            const r = switch (right) {
                .word => |v| @as(float, @floatFromInt(v)),
                .sword => |v| @as(float, @floatFromInt(v)),
                .float => |v| v,
                .str => |v| blk: {
                    if (std.fmt.parseFloat(float, v)) |parsed| {
                        break :blk parsed;
                    } else |_| {
                        return error.NotNumeric;
                    }
                },
            };

            return switch (op) {
                .add => .{ .float = l + r },
                .sub => .{ .float = l - r },
                .mul => .{ .float = l * r },
                .div => .{ .float = l / r },
                .num_eq => .{ .word = if (l == r) 0 else 1 },
                .num_ne => .{ .word = if (l != r) 0 else 1 },
                .gt => .{ .word = if (l > r) 0 else 1 },
                .lt => .{ .word = if (l < r) 0 else 1 },
                else => unreachable,
            };
        },
    };
}

fn reducePass(tier: wstr, operands: []const Operand) EvalError![]Operand {
    const allocator = mem.temp();
    var out: std.ArrayList(Operand) = .empty;
    var i: word = 0;

    while (i < operands.len) {
        const is_reducible_op =
            i > 0 and
            i + 1 < operands.len and
            operands[i] == .token and
            isOperator(operands[i].token) and
            inTier(tier, operands[i].token);

        if (is_reducible_op) {
            const left = resolveOperand(out.pop().?);
            const right = resolveOperand(operands[i + 1]);
            const op = op_kind.get(operands[i].token).?;
            const result = try applyOp(op, left, right);

            try out.append(allocator, .{ .value = result });
            i += 2;
        } else {
            try out.append(allocator, operands[i]);
            i += 1;
        }
    }

    return try out.toOwnedSlice(allocator);
}

pub fn evaluate(line: str) !state.Value {
    defer mem.resetTemp();

    var token_list: std.ArrayList(str) = .empty;
    var it = std.mem.splitScalar(byte, line, ' ');
    while (it.next()) |tok| {
        try token_list.append(mem.temp(), tok);
    }

    if (token_list.items.len == 1) {
        return toScratch(resolveValue(token_list.items[0]));
    }

    if (token_list.items.len < 3 or !isOperator(token_list.items[1])) {
        return toScratch(.{ .str = try resolveVariables(line) });
    }

    var operand_list: std.ArrayList(Operand) = .empty;
    for (token_list.items) |tok| {
        try operand_list.append(mem.temp(), .{ .token = tok });
    }

    var current: []Operand = operand_list.items;

    for (tiers) |tier| {
        current = reducePass(tier, current) catch {
            return toScratch(.{ .str = try resolveVariables(line) });
        };
    }

    if (current.len != 1) {
        return toScratch(.{ .str = try resolveVariables(line) });
    }

    return toScratch(resolveOperand(current[0]));
}

fn debugDump(instr: Instruction) !void { // debugger
    const capacity = mem.queryCapacity();
    const allocator = mem.persistent();

    print("\n======== VM DEBUG ========\n", .{});
    print("Instruction: {s} dest=({s},{s},expr={}) data=({s},{s},expr={}) has_data={}\n", .{
        @tagName(instr.op),
        instr.dest_text,
        @tagName(instr.dest_mode),
        instr.dest_expr != null,
        instr.data_text,
        @tagName(instr.data_mode),
        instr.data_expr != null,
        instr.has_data,
    });

    print("CODE:\n", .{});
    for (state.code.items) |item| {
        print("  {s} dest=({s},{s}) data=({s},{s})\n", .{
            @tagName(item.op),
            item.dest_text,
            @tagName(item.dest_mode),
            item.data_text,
            @tagName(item.data_mode),
        });
    }

    print("Recording: {}\n", .{recording});
    print("Recording depth: {}\n", .{recording_depth});

    print("CODE TABLE:\n", .{});
    var code_iter = state.codeTable.iterator();
    while (code_iter.next()) |entry| {
        print("  {s} = {}\n", .{ entry.key_ptr.*, entry.value_ptr.* });
    }

    print("DATA:\n", .{});
    var data_iter = state.data.iterator();
    while (data_iter.next()) |entry| {
        const value_string = try valueToString(mem.temp(), entry.value_ptr.*);
        print("  {s} = {s} ({s})\n", .{ entry.key_ptr.*, value_string, @tagName(entry.value_ptr.*) });
    }

    print("ESCAPES:\n", .{});
    escapes.dump();

    const cstring = try std.fmt.allocPrint(allocator, "{}", .{capacity / 1024 / 1024});
    print("MEM: {s}MB\n", .{cstring});
    print("STACK: {}\n", .{callStack.items.len});
    print("\n======== VM DEBUG ========\n", .{});
}

// end of codebase :D
