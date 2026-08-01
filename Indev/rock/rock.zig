const std = @import("std");
const platform = @import("platform");

const stdlib =
    \\. # rock stdlib
    \\fn print ( value ) value
    \\. New __Return_std.io.print_ARG0 'value'
    \\. Escape std.io.print
    \\}
    \\. # end rock stdlib
    \\
    \\
;

// errors
const Error = struct {
    kind: ErrorKind,
    line: isize,
    message: ?[]const u8,
};

const ErrorKind = enum {
    InvalidSyntax,
    UsageError
};

fn unwindExpression(
    allocator: std.mem.Allocator,
    expr: []const u8,
    funcname: []const u8,
) ![]const u8 {
    var tokens = std.mem.splitScalar(u8, expr, ' ');
    var output: std.ArrayList(u8) = .empty;
    var first = true;

    while (tokens.next()) |token| {
        if (!first)
            try output.append(allocator, ' ');

        first = false;

        if (std.mem.eql(u8, token, "+") or
            std.mem.eql(u8, token, "-") or
            std.mem.eql(u8, token, "*") or
            std.mem.eql(u8, token, "/"))
        {
            try output.appendSlice(allocator, token);
        } else if (std.fmt.parseInt(i64, token, 10) catch null != null) {
            try output.appendSlice(allocator, token);
        } else {
            const varname = try std.fmt.allocPrint(
                allocator,
                "__Func_{s}_{s}",
                .{ funcname, token },
            );

            try output.appendSlice(allocator, varname);
        }
    }

    return output.toOwnedSlice(allocator);
}

pub fn main() !void {
    if (try run()) |err| {
        std.debug.print(
            "Error: {any}: Line {d}: \n{s}\n",
            .{ 
                err.kind, 
                err.line, 
                err.message orelse "No message was given." },
        );
        return;
    }
}

fn run() !?Error {
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    const allocator = arena.allocator();
//    const allocator = std.heap.page_allocator;
    defer arena.deinit();

    var _args = std.process.args();
    _ = _args.next(); // Skip program name
    const first_arg = _args.next() orelse return Error{
        .kind = .UsageError,
        .line = 0,
        .message = "Missing first argument",
    };

    const data = try std.fs.cwd().readFileAlloc(
        allocator,
        first_arg,
        1024 * 1024 * 512,
    );
    defer allocator.free(data);

    const code = try std.mem.concat(
        allocator,
        u8,
        &.{ stdlib, data },
//        &.{  data },
    );

    var lines = std.mem.splitScalar(u8, code, '\n'); // split source code 
                                                     // into lines
    var output: []const u8 = undefined; // area for compiled output
    var funcname: []const u8 = undefined; // to tell others the name of the 
                                          // function
                                          // we are currently defining if 
                                          // applicable
    var recording: bool = false; // to tell others we are in a function 
                                 // definition
    var pending: []const u8 = undefined;
    const stdlib_line_count = std.mem.count(u8, stdlib, "\n");
    var line_num: isize = -@as(isize, @intCast(stdlib_line_count));

    while (lines.next()) |line| { 
        line_num += 1;
        if (line.len == 0) // check if the line is empty
            continue; // skip if so

        var inside: bool = false; // define for later
        var tokens: std.ArrayList([]const u8) = .empty; // define for later
        defer tokens.deinit(allocator); // defer to the allocator

        var tokensAsString = std.mem.splitScalar(u8, line, ' '); // separate
        while (tokensAsString.next()) |tokenAsString| { // for each item in 
                                                        // string
            try tokens.append(allocator, tokenAsString); // append to array
        }

//        for (tokens.items) |token|
//            std.debug.print("{s}\n", .{token});

        if (tokens.items[0][0] == '/') // check if is comment
            continue; // skip if so

        if (std.mem.eql(u8, tokens.items[0], "fn")) { // function
            if (tokens.items.len < 5) // check if short
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = 
                        \\Function is too short. Functions use the syntax of 
                        \\fn <name> ( <args> ) <return> { <body> }
                    ,
                };

            recording = true; // tell other things we are in a function 
                              // definition
            funcname = tokens.items[1]; // set name of function
            var args: std.ArrayList([]const u8) = .empty; // set for later use

            for (tokens.items) |token| {
                if (std.mem.eql(u8, token, "(")) { // check if start of args
                    inside = true; // tell others we are in the args
                    continue;
                } else if (std.mem.eql(u8, token, ")")) { // check if end of args
                    inside = false; // tell others we have left args
                    break;
                }

                if (inside)
                    try args.append(allocator, token);
            }

            var buffer: std.ArrayList(u8) = .empty; // set for later use
            for (args.items, 0..) |arg, z| {
                const arguments: []const u8 = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} __Func_{s}_ARG{d}\n",
                    .{ tokens.items[1], arg, tokens.items[1], z },
                );
                try buffer.appendSlice(allocator, arguments);
            }

            var return_value: []const u8 = "none";
            for (tokens.items, 0..) |token, w| {
                if (std.mem.eql(u8, token, ")")) {
                    if (w + 1 < tokens.items.len) 
                        return_value = tokens.items[w + 1];
                    break;
                }
            }

            pending = return_value;
            output = try std.fmt.allocPrint(
                allocator,
                "Func __Func_{s}\n{s}\n",
                .{ tokens.items[1], buffer.items },
            );
        } else if (std.mem.eql(u8, tokens.items[0], "}")) {
            output = try std.fmt.allocPrint(
                allocator, 
                "New __Func_{s}_RET0 __Func_{s}_{s}\nEnd\n",
                .{ funcname, funcname, pending },
            );
            
            recording = false;
        } else if (std.mem.eql(u8, tokens.items[0], "export")) {
            output = try std.fmt.allocPrint(
                allocator,
                "New {s} __Func_{s}_{s}\n",
                .{ tokens.items[1], funcname, tokens.items[1] },
            );
        } else if (std.mem.eql(u8, tokens.items[0], "import")) {
            output = try std.fmt.allocPrint(
                allocator,
                "New __Func_{s}_{s} {s}\n",
                .{ tokens.items[1], funcname, tokens.items[1] },
            );
    } else if (std.mem.eql(u8, tokens.items[1], "=")) {
        const omit = tokens.items[2][0];
        const first = tokens.items[2][1..];

        var value_tokens = tokens.items[2..];
        value_tokens[0] = first;

        const rest = try std.mem.join(
            allocator,
            " ",
            value_tokens,
        );

        if (recording) {
            if (omit == '`') {
                const unwinded = try unwindExpression(
                    allocator,
                    rest,
                    funcname,
                );

                output = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} \"{s}\"\n",
                    .{ funcname, tokens.items[0], unwinded },
                );
            } else if (omit == '!') {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} '{s}'\n",
                    .{ funcname, tokens.items[0], rest },
                );
            } else if (omit == '@') {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} __Func_{s}_{s}\n",
                    .{ funcname, tokens.items[0], funcname, rest },
                );
            } else if (omit == ':') {
                var args: std.ArrayList([]const u8) = .empty;
                var buffer: std.ArrayList(u8) = .empty;
                var temp: bool = false;

                for (tokens.items) |token| {
                    if (std.mem.eql(u8, token, "(")) {
                        temp = true;
                        continue;
                    } else if (std.mem.eql(u8, token, ")")) {
                        temp = false;
                        break;
                    }

                    if (temp)
                        try args.append(allocator, token);
                }

                for (args.items, 0..) |arg, num| {
                    const thing = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} __Func_{s}_{s}\n",
                        .{ tokens.items[2], num, funcname, arg },
                    );

                    try buffer.appendSlice(allocator, thing);
                }

                const ret = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} __Func_{s}_RET0\n",
                    .{ funcname, tokens.items[0], tokens.items[2] },
                );

                output = try std.fmt.allocPrint(
                    allocator,
                    "{s}Call __Func_{s}\n{s}\n",
                    .{ buffer.items, tokens.items[2], ret },
                );
            } else {
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message =
                        \\Expression type is invalid.
                        \\Supported types include
                        \\! for literals,
                        \\` for expressions, and
                        \\@ for copying values.
                        \\The expression type marker should be the first
                        \\character of the expression, eg.
                        \\x = !5
                    ,
                };
            }
        } else {
            if (omit == '`') {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New {s} \"{s}\"\n",
                    .{ tokens.items[0], rest },
                );
            } else if (omit == '!') {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New {s} '{s}'\n",
                    .{ tokens.items[0], rest },
                );
            } else if (omit == '@') {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New {s} {s}\n",
                    .{ tokens.items[0], rest },
                );
            } else if (omit == ':') {
                var args: std.ArrayList([]const u8) = .empty;
                var buffer: std.ArrayList(u8) = .empty;
                var temp: bool = false;

                for (tokens.items) |token| {
                    if (std.mem.eql(u8, token, "(")) {
                        temp = true;
                        continue;
                    } else if (std.mem.eql(u8, token, ")")) {
                        temp = false;
                        break;
                    }

                    if (temp)
                        try args.append(allocator, token);
                }

                for (args.items, 0..) |arg, num| {
                    const thing = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} '{s}'\n",
                        .{ tokens.items[2], num, arg },
                    );

                    try buffer.appendSlice(allocator, thing);
                }

                const ret = try std.fmt.allocPrint(
                    allocator,
                    "New {s} __Func_{s}_RET0\n",
                    .{ tokens.items[0], tokens.items[2] },
                );

                output = try std.fmt.allocPrint(
                    allocator,
                    "{s}Call __Func_{s}\n{s}\n",
                    .{ buffer.items, tokens.items[2], ret },
                );
            } else {
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message =
                        \\Expression type is invalid.
                    ,
                };
            }
        }

    } else if (std.mem.eql(u8, tokens.items[0], "call")) {
        var args: std.ArrayList([]const u8) = .empty;
        var buffer: std.ArrayList(u8) = .empty;
        var temp: bool = false;

        for (tokens.items) |token| {
            if (std.mem.eql(u8, token, "(")) {
                temp = true;
                continue;
            } else if (std.mem.eql(u8, token, ")")) {
                temp = false;
                break;
            }

            if (temp)
                try args.append(allocator, token);
        }

        for (args.items, 0..) |arg, num| {
            const thing = if (recording)
                try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_ARG{d} __Func_{s}_{s}\n",
                    .{ tokens.items[1], num, funcname, arg },
                )
            else
                try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_ARG{d} '{s}'\n",
                    .{ tokens.items[1], num, arg },
                );

            try buffer.appendSlice(allocator, thing);
        }

        output = try std.fmt.allocPrint(
            allocator,
            "{s}Call __Func_{s}\n",
            .{ buffer.items, tokens.items[1] },
        );
        } else if (std.mem.eql(u8, tokens.items[0], "call")) { 
            // call funcname ( arg1 arg2 )

            var args: std.ArrayList([]const u8) = .empty;
            var buffer: std.ArrayList(u8) = .empty;
            var temp: bool = false;

            // First pass: collect arguments
            for (tokens.items) |token| {
                if (std.mem.eql(u8, token, "(")) {
                    temp = true;
                    continue;
                } else if (std.mem.eql(u8, token, ")")) {
                    temp = false;
                    break;
                }

                if (temp)
                    try args.append(allocator, token);
            }

            // Second pass: generate argument setup
            for (args.items, 0..) |arg, num| {
                var thing: []const u8 = undefined;

                if (recording) {
                    thing = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} __Func_{s}_{s}\n",
                        .{ tokens.items[1], num, funcname, arg },
                    );
                } else {
                    thing = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} {s}\n",
                        .{ tokens.items[1], num, arg },
                    );
                }

                try buffer.appendSlice(allocator, thing);
            }
            output = try std.fmt.allocPrint(
                allocator,
                "{s}Call __Func_{s}\n",
                .{ buffer.items, tokens.items[1] },
            );
        } else if (std.mem.eql(u8, tokens.items[0], ".")) {
            const rest = try std.mem.join(
                allocator,
                " ",
                tokens.items[1..],
            );

            const injected = try std.fmt.allocPrint(
                allocator,
                "{s} #  --- INJECTED ---",
                .{rest},
            );

            output = injected;
        } else 
//            return Error{
//                .kind = .InvalidSyntax,
//                .line = line_num,
//                .message = "Invalid operation.",
//            };
            continue;
        
        var out = std.mem.splitScalar(u8, output, '\n');
        while (out.next()) |outline| {
            if (outline.len == 0) // remove newlines
                continue;
            platform.print("{s}\n", .{outline}); // just print for now
        }
        
//        platform.print("{s}\n", .{output}); // just print for now
    }
    return null;
}
