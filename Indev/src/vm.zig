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

var first_arg: ?[]const u8 = null;
var second_arg: ?[]const u8 = null;

// for windows compat
fn getArgs(allocator: std.mem.Allocator) ![][]const u8 {
    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    var list: std.ArrayList([]const u8) = .empty;

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
    state.data = std.StringHashMap([]const u8).init(allocator);
    state.code = .empty;
    state.codeTable = std.StringHashMap(usize).init(allocator);

    defer state.data.deinit();
    defer state.code.deinit(allocator);
    defer state.codeTable.deinit();

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
        if (std.mem.eql(u8, arg, "version")) {
            print("{s}\n", .{version.full});
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle tests
        if (std.mem.eql(u8, arg, "test")) {
            try run(tests.tests);
            exit(0);
        }
    }

    if (first_arg) |arg| { // handle docs
        if (std.mem.eql(u8, arg, "docs")) {
            print("{s}", .{docs.read()});
            exit(0);
        }
    }
    
    if (first_arg) |arg| { // handle shell
        if (std.mem.eql(u8, arg, "psh")) {
            try run(psh.psh);
            exit(0);
        }
    }


    const filename = first_arg orelse { return; };
    const fileData = try readFile(filename);
    try run(fileData);
}

pub fn run(fileData: []const u8) !void {
    const allocator = mem.alloc();
    var lines = std.mem.splitScalar(u8, fileData, '\n');

    while (lines.next()) |line| {
        if (line.len == 0) continue;
        const IR = try tokenize(line);
        if (IR.len == 0) continue; // skip newlines and comments and such
        defer allocator.free(IR);
        if (build.VMDEBUG) try debugDump(IR); // debug dump
        try interpret(IR);
    }

//    defer allocator.free(fileData);
}

// read a file
fn readFile(fileName: []const u8) ![]u8 {
    const allocator = mem.alloc();
    return try std.fs.cwd().readFileAlloc(
        allocator,
        fileName,
        512 * 1024 * 1024,
    );
}

fn tokenize(line: []const u8) ![][]const u8 {
    const allocator = mem.alloc(); 
    var tokens: std.ArrayList([]const u8) = .empty;
    errdefer tokens.deinit(allocator);

    var i: usize = 0;

    while (i < line.len) {
        while (i < line.len and std.ascii.isWhitespace(line[i])) { // skip whitespace
            i += 1;
        }

        if (i >= line.len) break;

        if (line[i] == '#') { // comment
            i += 1;
            break;
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
    }
    return try tokens.toOwnedSlice(allocator);
}

fn findEscape(name: []const u8) ?escapes.Escape {
    for (escapes.table) |escape| {
        if (std.mem.eql(u8, escape.name, name)) {
            return escape;
        }
    }

    return null;
}

fn interpret(list: [][]const u8) !void {
    const allocator = mem.alloc();
    
    if (limits.inst_curr > limits.inst_max) {
        print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
        exit(1);
    }

    limits.inst_curr += 1;

    if (recording) {
        if (std.mem.eql(u8, list[0], "End")) {
            try state.code.append(allocator, try copyInstruction(list));
            recording = false;
            return;
        } else {
            try state.code.append(allocator, try copyInstruction(list));
            return;
        }
    }

    if (std.mem.eql(u8, list[0], "New")) { // New
        if (limits.inst_new_curr > limits.inst_new_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_new_curr += 1;

        const newData = try evaluate(list[2]); // do math
        var newList = try allocator.alloc([]const u8, list.len);
        @memcpy(newList, list);
        defer allocator.free(newList);
        newList[2] = newData;

        if (list.len > 3) {
            if (std.mem.eql(u8, list[3], "///")) { // true literal
                try state.data.put(list[1], list[2]);
            } else if (std.mem.eql(u8, list[3], "////")) { // force eval
                const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
                const data = try evaluate(indirect);
                try state.data.put(list[1], data);
            } else if (std.mem.eql(u8, list[3], "//")) { // literal
                try state.data.put(newList[1], newList[2]);
            }
            
        } else { // indirect
            const indirect = state.data.get(list[2]) orelse return error.UnknownVariable;
            try state.data.put(list[1], indirect);
        }
    }

    if (std.mem.eql(u8, list[0], "Escape")) { // Escape
        if (limits.inst_escape_curr > limits.inst_escape_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_escape_curr += 1;


        const escape = findEscape(list[1]) orelse return error.UnknownEscape;
        try escape.run();
    }

    if (std.mem.eql(u8, list[0], "Func")) { // Start recording functions
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_func_curr += 1;


        recording = true;
        try state.code.append(allocator, try copyInstruction(list));
        try state.codeTable.put(list[1], state.code.items.len);        
    }

    if (std.mem.eql(u8, list[0], "Call")) { // Call a recorded function
        if (limits.inst_call_curr > limits.inst_call_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_call_curr += 1;

        try callFunc(list);
    }
    
    if (std.mem.eql(u8, list[0], "If")) { // Start doing Ifs (If funcToExec "condition")
        if (limits.inst_func_curr > limits.inst_func_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_func_curr += 1;

        if (std.mem.eql(u8, try evaluate(list[2]), "0")) {
            try callFunc(list);
        }
    }

    if (std.mem.eql(u8, list[0], "Return")) { // thing to exit from Func
        if (limits.inst_return_curr > limits.inst_return_max) {
            print("VM: FATAL: INSTRUCTION LIMIT REACHED\n", .{});
            exit(1);
        }

        limits.inst_return_curr += 1;


        return error.Return;
    }
} 

fn callFunc(list: [][]const u8) anyerror!void {
    const recording_old = recording;
    recording = false;

    const start = state.codeTable.get(list[1]) orelse return error.UnknownFunction;
    var pc = start;

    while (true) {
        const instruction = state.code.items[pc];

        if (std.mem.eql(u8, instruction[0], "End")) break;

        interpret(instruction) catch |err| {
            if (err == error.Return) {
                break;
            }
            return err;
        };
        pc += 1;
    }
    recording = recording_old;
}



fn evaluate(line: []const u8) ![]const u8 { // WARNING - This WILL fail silently
    const allocator = mem.alloc();
    var parts = std.mem.splitScalar(u8, line, ' ');
    var left_str = parts.next() orelse return line;
    const op = parts.next() orelse return line;
    var right_str = parts.next() orelse return line;  

    const left_str_data = state.data.get(left_str);
    const right_str_data = state.data.get(right_str);

    if (left_str_data) |data| {
        left_str = data;
    }

    if (right_str_data) |data| {
        right_str = data;
    }
   
    if (std.mem.eql(u8, op, "s++")) {
        return try std.fmt.allocPrint(
            allocator, 
            "{s}{s}",
            .{ left_str, right_str },
        );
    }

    if (std.mem.eql(u8, op, "?=")) {
        var result: []const u8 = undefined;

        if (std.mem.eql(u8, left_str, right_str)) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, "s?=")) {
        var result: []const u8 = undefined;

        if (std.mem.startsWith(u8, left_str, right_str)) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, "e?=")) {
        var result: []const u8 = undefined;

        if (std.mem.endsWith(u8, left_str, right_str)) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, "-?=")) {
        var result: []const u8 = undefined;

        if (std.mem.indexOf(u8, left_str, right_str) != null) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }


//    var left: ??? = undefined;
//    var right: ??? = undefined;

//    if (std.fmt.parseInt(i32, left_str, 10)) |i| {
//        left = i;
//    } else |_| {
//        if (std.fmt.parseFloat(f32, left_str)) |f| {
//            left = f;
//        } else {
//            return;
//        }
//    }
    
//    if (std.fmt.parseInt(i32, left_str, 10)) |i| {
//        left = i;
//    } else |_| {
//        if (std.fmt.parseFloat(f32, left_str)) |f| {
//            left = f;
//        } else {
//            return;
//        }
//    }



//    const right = std.fmt.parseInt(i32, right_str, 10) catch return line;
//    const left = std.fmt.parseInt(i32, left_str, 10) catch return line;

    const right = std.fmt.parseFloat(f32, right_str) catch return line; 
    const left = std.fmt.parseFloat(f32, left_str) catch return line;



    if (std.mem.eql(u8, op, "+")) {
        return try std.fmt.allocPrint(
            allocator,
            "{}",
            .{left + right},
        );
    }

    if (std.mem.eql(u8, op, "-")) {
        return try std.fmt.allocPrint(
            allocator,
            "{}",
            .{left - right},
        );
    }

    if (std.mem.eql(u8, op, "*")) {
        return try std.fmt.allocPrint(
            allocator,
            "{}",
            .{left * right},
        );
    }

    if (std.mem.eql(u8, op, "/")) {
        return try std.fmt.allocPrint(
            allocator,
            "{}",
            .{@divTrunc(left, right)},
        );
    }

    if (std.mem.eql(u8, op, "==")) {
        var result: []const u8 = undefined;

        if (left == right) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, "!=")) {
        var result: []const u8 = undefined;

        if (left != right) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, ">")) {
        var result: []const u8 = undefined;

        if (left > right) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    if (std.mem.eql(u8, op, "<")) {
        var result: []const u8 = undefined;

        if (left < right) {
            result = "0";
        } else {
            result = "1";
        }

        return try std.fmt.allocPrint(
            allocator,
            "{s}",
            .{result},
        );
    }

    return line;
}

fn copyInstruction(list: [][]const u8) ![][]const u8 {
    const allocator = mem.alloc();
    var copy = try allocator.alloc([]const u8, list.len);

    for (list, 0..) |token, i| {
        copy[i] = try allocator.dupe(u8, token);
    }

    return copy;
}

fn debugDump(ir: [][]const u8) !void {
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

    print("\n======== VM DEBUG ========\n", .{});
}
