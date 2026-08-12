const std = @import("std");
const platform = @import("platform");
const stdlib = @embedFile("stdlib");

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
                err.message orelse "No message was given.",
            },
        );
        return;
    }
}

fn run() !?Error {
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    const allocator = arena.allocator();
    defer arena.deinit();

    var _args = std.process.args();
    _ = _args.next();

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
    );

    var lines = std.mem.splitScalar(u8, code, '\n');

    var output: []const u8 = undefined;

    var funcname: []const u8 = undefined;
    var recording: bool = false;

    var pending: []const u8 = undefined;

    const stdlib_line_count = std.mem.count(u8, stdlib, "\n");
    var line_num: isize = -@as(isize, @intCast(stdlib_line_count));

    // if state
    var in_if: bool = false;
    var if_funcname: []const u8 = undefined;
    var if_condition: []const u8 = undefined;
    var if_counter: usize = 0;
    var if_buffer: std.ArrayList(u8) = .empty;
    defer if_buffer.deinit(allocator);

    while (lines.next()) |line| {
        line_num += 1;

        if (line.len == 0)
            continue;

        var inside: bool = false;

        var tokens: std.ArrayList([]const u8) = .empty;
        defer tokens.deinit(allocator);

        var tokensAsString = std.mem.splitScalar(u8, line, ' ');

        while (tokensAsString.next()) |tokenAsString| {
            try tokens.append(allocator, tokenAsString);
        }

        if (tokens.items.len == 0)
            continue;

        if (tokens.items[0].len == 0)
            continue;

        if (tokens.items[0][0] == '/')
            continue;

        //
        // IF
        //
        if (std.mem.eql(u8, tokens.items[0], "if")) {
            if (tokens.items.len < 5 or
                !std.mem.eql(u8, tokens.items[1], "(") or
                !std.mem.eql(u8, tokens.items[tokens.items.len - 2], ")") or
                !std.mem.eql(u8, tokens.items[tokens.items.len - 1], "{"))
            {
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message =
                        \\Invalid if syntax.
                        \\If statements use:
                        \\if ( <expression> ) {
                    ,
                };
            }

            if (in_if) {
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = "Nested if statements are not supported yet.",
                };
            }

            if_funcname = try std.fmt.allocPrint(
                allocator,
                "__RockIf_{d}",
                .{if_counter},
            );

            if_counter += 1;

            if_condition = try std.mem.join(
                allocator,
                " ",
                tokens.items[2 .. tokens.items.len - 2],
            );

            in_if = true;
            if_buffer.clearRetainingCapacity();

            continue;
        }

        //
        // END OF BLOCK
        //
        if (std.mem.eql(u8, tokens.items[0], "}")) {
            if (in_if) {
                output = try std.fmt.allocPrint(
                    allocator,
                    "Func {s}\n{s}End\nIf {s} \"{s}\"\n",
                    .{
                        if_funcname,
                        if_buffer.items,
                        if_funcname,
                        if_condition,
                    },
                );

                in_if = false;
            } else {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_RET0 __Func_{s}_{s}\nEnd\n",
                    .{ funcname, funcname, pending },
                );

                recording = false;
            }

            if (in_if) {
                try if_buffer.appendSlice(allocator, output);
            } else {
                var out = std.mem.splitScalar(u8, output, '\n');

                while (out.next()) |outline| {
                    if (outline.len == 0)
                        continue;

                    platform.print("{s}\n", .{outline});
                }
            }

            continue;
        }

        //
        // FUNCTION
        //
        if (std.mem.eql(u8, tokens.items[0], "fn")) {
            if (tokens.items.len < 5)
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message =
                        \\Function is too short. Functions use the syntax of
                        \\fn <name> ( <args> ) <return> { <body> }
                    ,
                };

            recording = true;
            funcname = tokens.items[1];

            var args: std.ArrayList([]const u8) = .empty;
            defer args.deinit(allocator);

            for (tokens.items) |token| {
                if (std.mem.eql(u8, token, "(")) {
                    inside = true;
                    continue;
                } else if (std.mem.eql(u8, token, ")")) {
                    inside = false;
                    break;
                }

                if (inside)
                    try args.append(allocator, token);
            }

            var buffer: std.ArrayList(u8) = .empty;
            defer buffer.deinit(allocator);

            for (args.items, 0..) |arg, z| {
                const arguments = try std.fmt.allocPrint(
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

        //
        // EXPORT
        //
        } else if (std.mem.eql(u8, tokens.items[0], "export")) {
            output = try std.fmt.allocPrint(
                allocator,
                "New {s} __Func_{s}_{s}\n",
                .{ tokens.items[1], funcname, tokens.items[1] },
            );

        //
        // IMPORT
        //
        } else if (std.mem.eql(u8, tokens.items[0], "import")) {
            output = try std.fmt.allocPrint(
                allocator,
                "New __Func_{s}_{s} {s}\n",
                .{ funcname, tokens.items[1], tokens.items[1] },
            );

        //
        // ASSIGNMENT
        //
        } else if (tokens.items.len > 1 and
            std.mem.eql(u8, tokens.items[1], "="))
        {
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
                    defer args.deinit(allocator);

                    var buffer: std.ArrayList(u8) = .empty;
                    defer buffer.deinit(allocator);

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
                    defer args.deinit(allocator);

                    var buffer: std.ArrayList(u8) = .empty;
                    defer buffer.deinit(allocator);

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

        //
        // CALL
        //
        } else if (std.mem.eql(u8, tokens.items[0], "call")) {
            var args: std.ArrayList([]const u8) = .empty;
            defer args.deinit(allocator);

            var buffer: std.ArrayList(u8) = .empty;
            defer buffer.deinit(allocator);

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

        //
        // RAW INJECTION
        //
        } else if (std.mem.eql(u8, tokens.items[0], ".")) {
            const rest = try std.mem.join(
                allocator,
                " ",
                tokens.items[1..],
            );

            output = try std.fmt.allocPrint(
                allocator,
                "{s} # --- INJECTED ---",
                .{rest},
            );

        } else {
            continue;
        }

        //
        // If we're currently recording an if body, store the generated
        // Pebble instructions instead of emitting them immediately.
        //
        if (in_if) {
            try if_buffer.appendSlice(allocator, output);
        } else {
            var out = std.mem.splitScalar(u8, output, '\n');

            while (out.next()) |outline| {
                if (outline.len == 0)
                    continue;

                platform.print("{s}\n", .{outline});
            }
        }
    }

    return null;
}
