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
    UsageError,
};

fn tokenizeWhitespace(
    allocator: std.mem.Allocator, 
    line: []const u8,
) !std.ArrayList([]const u8) {
    var tokens: std.ArrayList([]const u8) = .empty;

    var start: usize = 0;
    var i: usize = 0;

    while (i < line.len) {
        const c = line[i];

        if (c == ' ' or c == '\t' or c == '\r') {
            if (start < i) {
                try tokens.append(
                    allocator,
                    line[start..i],
                );
            }

            i += 1;

            while (i < line.len and
                (line[i] == ' ' or
                    line[i] == '\t' or
                    line[i] == '\r'))
            {
                i += 1;
            }

            start = i;
            continue;
        }

        i += 1;
    }

    if (start < line.len) {
        try tokens.append(
            allocator,
            line[start..],
        );
    }

    return tokens;
}

fn unwindExpression(
    allocator: std.mem.Allocator,
    expr: []const u8,
    funcname: []const u8,
) ![]const u8 {
    var tokens = try tokenizeWhitespace(
        allocator,
        expr,
    );
    defer tokens.deinit(allocator);

    var output: std.ArrayList(u8) = .empty;
    var first_token = true;

    for (tokens.items) |token| {
        if (!first_token)
            try output.append(allocator, ' ');

        first_token = false;

        if (std.mem.eql(u8, token, "+") or
            std.mem.eql(u8, token, "-") or
            std.mem.eql(u8, token, "*") or
            std.mem.eql(u8, token, "/") or
            std.mem.eql(u8, token, "==") or
            std.mem.eql(u8, token, "!=") or
            std.mem.eql(u8, token, ">") or
            std.mem.eql(u8, token, "<") or
            std.mem.eql(u8, token, "e?=") or
            std.mem.eql(u8, token, "s?=") or
            std.mem.eql(u8, token, "?=") or
            std.mem.eql(u8, token, "s++") or
            std.mem.eql(u8, token, "-?="))
        {
            try output.appendSlice(
                allocator,
                token,
            );
        } else if (std.fmt.parseInt(
            i64,
            token,
            10,
        ) catch null != null) {
            try output.appendSlice(
                allocator,
                token,
            );
        } else {
            const varname = try std.fmt.allocPrint(
                allocator,
                "__Func_{s}_{s}",
                .{
                    funcname,
                    token,
                },
            );

            try output.appendSlice(
                allocator,
                varname,
            );
        }
    }

    return output.toOwnedSlice(allocator);
}

const IfBlock = struct {
    funcname: []const u8,
    condition: []const u8,
    buffer: std.ArrayList(u8),
};

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

    const code = try std.mem.concat(
        allocator,
        u8,
        &.{ stdlib, data },
    );

    var lines = std.mem.splitScalar(
        u8,
        code,
        '\n',
    );

    var output: []const u8 = undefined;
    var funcname: []const u8 = undefined;
    var recording: bool = false;
    var pending: []const u8 = undefined;
    const stdlib_line_count = std.mem.count(u8, stdlib, "\n");
    var line_num: isize = -@as(isize, @intCast(stdlib_line_count));
    var if_counter: usize = 0;
    var if_stack: std.ArrayList(IfBlock) = .empty;

    defer {
        for (if_stack.items) |*block| {
            block.buffer.deinit(allocator);
        }

        if_stack.deinit(allocator);
    }

    while (lines.next()) |line| {
        line_num += 1;

        if (line.len == 0)
            continue;

        var tokens = try tokenizeWhitespace(
            allocator,
            line,
        );

        defer tokens.deinit(allocator);

        if (tokens.items.len == 0)
            continue;

        if (tokens.items[0].len == 0)
            continue;

        if (tokens.items[0][0] == '/')
            continue;

        var inside: bool = false;

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

            const new_funcname = try std.fmt.allocPrint(
                allocator,
                "__RockIf_{d}",
                .{if_counter},
            );

            if_counter += 1;

            const condition = try std.mem.join(
                allocator,
                " ",
                tokens.items[2 .. tokens.items.len - 2],
            );

            try if_stack.append(
                allocator,
                .{
                    .funcname = new_funcname,
                    .condition = condition,
                    .buffer = .empty,
                },
            );

            continue;
        }

        if (std.mem.eql(u8, tokens.items[0], "}")) {
            if (if_stack.items.len > 0) {
                var completed = if_stack.pop().?;
                const scoped_name = completed.funcname;

                output = try std.fmt.allocPrint(
                    allocator,
                    "Func {s}\n{s}End\nIf {s} \"{s}\"\n",
                    .{
                        scoped_name,
                        completed.buffer.items,
                        scoped_name,
                        completed.condition,
                    },
                );

                completed.buffer.deinit(allocator);

                if (if_stack.items.len > 0) {
                    try if_stack.items[
                        if_stack.items.len - 1
                    ].buffer.appendSlice(
                        allocator,
                        output,
                    );
                } else {
                    var out = std.mem.splitScalar(
                        u8,
                        output,
                        '\n',
                    );

                    while (out.next()) |outline| {
                        if (outline.len == 0)
                            continue;

                        platform.print(
                            "{s}\n",
                            .{outline},
                        );
                    }
                }

                continue;
            }

            if (recording) {
                output = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_RET0 __Func_{s}_{s}\nEnd\n",
                    .{
                        funcname,
                        funcname,
                        pending,
                    },
                );

                recording = false;

                var out = std.mem.splitScalar(
                    u8,
                    output,
                    '\n',
                );

                while (out.next()) |outline| {
                    if (outline.len == 0)
                        continue;

                    platform.print(
                        "{s}\n",
                        .{outline},
                    );
                }

                continue;
            }

            return Error{
                .kind = .InvalidSyntax,
                .line = line_num,
                .message = "Unexpected '}'.",
            };
        }

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
                }

                if (std.mem.eql(u8, token, ")")) {
                    inside = false;
                    break;
                }

                if (inside)
                    try args.append(
                        allocator,
                        token,
                    );
            }

            var buffer: std.ArrayList(u8) = .empty;
            defer buffer.deinit(allocator);

            for (args.items, 0..) |arg, z| {
                const arguments = try std.fmt.allocPrint(
                    allocator,
                    "New __Func_{s}_{s} __Func_{s}_ARG{d}\n",
                    .{
                        tokens.items[1],
                        arg,
                        tokens.items[1],
                        z,
                    },
                );

                try buffer.appendSlice(
                    allocator,
                    arguments,
                );
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
                .{
                    tokens.items[1],
                    buffer.items,
                },
            );

            var out = std.mem.splitScalar(u8, output, '\n');

            while (out.next()) |outline| {
                if (outline.len == 0)
                    continue;

                platform.print(
                    "{s}\n",
                    .{outline},
                );
            }

            continue;
        }

        if (std.mem.eql(u8, tokens.items[0], "export")) {
            if (tokens.items.len != 2)
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = "Export expects exactly one variable.",
                };

            output = try std.fmt.allocPrint(
                allocator,
                "New {s} __Func_{s}_{s}\n",
                .{
                    tokens.items[1],
                    funcname,
                    tokens.items[1],
                },
            );

            if (if_stack.items.len > 0) {
                try if_stack.items[
                    if_stack.items.len - 1
                ].buffer.appendSlice(
                    allocator,
                    output,
                );
            } else {
                platform.print(
                    "{s}",
                    .{output},
                );
            }

            continue;
        }

        if (std.mem.eql(u8, tokens.items[0], "import")) {
            if (tokens.items.len != 2)
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = "Import expects exactly one variable.",
                };

            output = try std.fmt.allocPrint(
                allocator,
                "New __Func_{s}_{s} {s}\n",
                .{
                    funcname,
                    tokens.items[1],
                    tokens.items[1],
                },
            );

            if (if_stack.items.len > 0) {
                try if_stack.items[
                    if_stack.items.len - 1
                ].buffer.appendSlice(
                    allocator,
                    output,
                );
            } else {
                platform.print(
                    "{s}",
                    .{output},
                );
            }

            continue;
        }

        if (tokens.items.len > 1 and
            std.mem.eql(u8, tokens.items[1], "=")) {
            if (tokens.items.len < 3)
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = "Assignment is missing a value.",
                };

            if (tokens.items[2].len == 0)
                return Error{
                    .kind = .InvalidSyntax,
                    .line = line_num,
                    .message = "Assignment is missing a value.",
                };

            const omit = tokens.items[2][0];
            const value_start = tokens.items[2][1..];

            var value_tokens = tokens.items[2..];
            value_tokens[0] = value_start;

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
                        .{
                            funcname,
                            tokens.items[0],
                            unwinded,
                        },
                    );
                } else if (omit == '!') {
                    output = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_{s} '{s}'\n",
                        .{
                            funcname,
                            tokens.items[0],
                            rest,
                        },
                    );
                } else if (omit == '@') {
                    output = try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_{s} __Func_{s}_{s}\n",
                        .{
                            funcname,
                            tokens.items[0],
                            funcname,
                            rest,
                        },
                    );
                } else if (omit == ':') {
                    var call_args: std.ArrayList([]const u8) = .empty;
                    defer call_args.deinit(allocator);
                    var arg_start: usize = 0;

                    for (value_tokens, 0..) |token, i| {
                        if (i == 0)
                            continue;

                        if (std.mem.eql(u8, token, "(")) {
                            arg_start = i + 1;
                            continue;
                        }

                        if (std.mem.eql(u8, token, ")"))
                            break;
                    }

                    if (arg_start == 0)
                        return Error{
                            .kind = .InvalidSyntax,
                            .line = line_num,
                            .message =
                                \\Function calls use:
                                \\result = :function ( arg1 arg2 )
                            ,
                        };

                    for (value_tokens[arg_start..]) |arg| {
                        if (std.mem.eql(u8, arg, ")"))
                            break;

                        try call_args.append(
                            allocator,
                            arg,
                        );
                    }

                    output = try std.fmt.allocPrint(
                        allocator,
                        "",
                        .{},
                    );

                    for (call_args.items, 0..) |arg, num| {
                        const argument = try std.fmt.allocPrint(
                            allocator,
                            "New __Func_{s}_ARG{d} __Func_{s}_{s}\n",
                            .{
                                value_start,
                                num,
                                funcname,
                                arg,
                            },
                        );

                        output = try std.fmt.allocPrint(
                            allocator,
                            "{s}{s}",
                            .{
                                output,
                                argument,
                            },
                        );
                    }

                    const call = try std.fmt.allocPrint(
                        allocator,
                        "Call __Func_{s}\nNew __Func_{s}_{s} __Func_{s}_RET0\n",
                        .{
                            value_start,
                            funcname,
                            tokens.items[0],
                            value_start,
                        },
                    );

                    output = try std.fmt.allocPrint(
                        allocator,
                        "{s}{s}",
                        .{
                            output,
                            call,
                        },
                    );
                } else {
                    return Error{
                        .kind = .InvalidSyntax,
                        .line = line_num,
                        .message =
                            \\Expression type is invalid.
                            \\Supported types include
                            \\! for literals,
                            \\` for expressions,
                            \\@ for copying values, and
                            \\: for function calls.
                        ,
                    };
                }
            } else {
                if (omit == '`') {
                    output = try std.fmt.allocPrint(
                        allocator,
                        "New {s} \"{s}\"\n",
                        .{
                            tokens.items[0],
                            rest,
                        },
                    );
                } else if (omit == '!') {
                    output = try std.fmt.allocPrint(
                        allocator,
                        "New {s} '{s}'\n",
                        .{
                            tokens.items[0],
                            rest,
                        },
                    );
                } else if (omit == '@') {
                    output = try std.fmt.allocPrint(
                        allocator,
                        "New {s} {s}\n",
                        .{
                            tokens.items[0],
                            rest,
                        },
                    );
                } else if (omit == ':') {
                    var call_args: std.ArrayList([]const u8) = .empty;
                    defer call_args.deinit(allocator);
                    var arg_start: usize = 0;

                    for (value_tokens, 0..) |token, i| {
                        if (i == 0)
                            continue;

                        if (std.mem.eql(u8, token, "(")) {
                            arg_start = i + 1;
                            continue;
                        }

                        if (std.mem.eql(u8, token, ")"))
                            break;
                    }

                    if (arg_start == 0)
                        return Error{
                            .kind = .InvalidSyntax,
                            .line = line_num,
                            .message =
                                \\Function calls use:
                                \\result = :function ( arg1 arg2 )
                            ,
                        };

                    for (value_tokens[arg_start..]) |arg| {
                        if (std.mem.eql(u8, arg, ")"))
                            break;

                        try call_args.append(
                            allocator,
                            arg,
                        );
                    }

                    output = try std.fmt.allocPrint(
                        allocator,
                        "",
                        .{},
                    );

                    for (call_args.items, 0..) |arg, num| {
                        const argument = try std.fmt.allocPrint(
                            allocator,
                            "New __Func_{s}_ARG{d} {s}\n",
                            .{
                                value_start,
                                num,
                                arg,
                            },
                        );

                        output = try std.fmt.allocPrint(
                            allocator,
                            "{s}{s}",
                            .{
                                output,
                                argument,
                            },
                        );
                    }

                    const call = try std.fmt.allocPrint(
                        allocator,
                        "Call __Func_{s}\nNew {s} __Func_{s}_RET0\n",
                        .{
                            value_start,
                            tokens.items[0],
                            value_start,
                        },
                    );

                    output = try std.fmt.allocPrint(
                        allocator,
                        "{s}{s}",
                        .{
                            output,
                            call,
                        },
                    );
                } else {
                    return Error{
                        .kind = .InvalidSyntax,
                        .line = line_num,
                        .message =
                            \\Expression type is invalid.
                            \\Supported types include
                            \\! for literals,
                            \\` for expressions,
                            \\@ for copying values, and
                            \\: for function calls.
                        ,
                    };
                }
            }
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
                }

                if (std.mem.eql(u8, token, ")")) {
                    temp = false;
                    break;
                }

                if (temp)
                    try args.append(
                        allocator,
                        token,
                    );
            }

            for (args.items, 0..) |arg, num| {
                const thing = if (recording)
                    try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} __Func_{s}_{s}\n",
                        .{
                            tokens.items[1],
                            num,
                            funcname,
                            arg,
                        },
                    )
                else
                    try std.fmt.allocPrint(
                        allocator,
                        "New __Func_{s}_ARG{d} '{s}'\n",
                        .{
                            tokens.items[1],
                            num,
                            arg,
                        },
                    );

                try buffer.appendSlice(
                    allocator,
                    thing,
                );
            }

            output = try std.fmt.allocPrint(
                allocator,
                "{s}Call __Func_{s}\n",
                .{
                    buffer.items,
                    tokens.items[1],
                },
            );
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

        if (if_stack.items.len > 0) {
            try if_stack.items[
                if_stack.items.len - 1
            ].buffer.appendSlice(
                allocator,
                output,
            );
        } else {
            var out = std.mem.splitScalar(u8, output, '\n');

            while (out.next()) |outline| {
                if (outline.len == 0)
                    continue;

                platform.print(
                    "{s}\n",
                    .{outline},
                );
            }
        }
    }

    if (if_stack.items.len != 0) {
        return Error{
            .kind = .InvalidSyntax,
            .line = line_num,
            .message = "Unclosed if statement.",
        };
    }

    if (recording) {
        return Error{
            .kind = .InvalidSyntax,
            .line = line_num,
            .message = "Unclosed function.",
        };
    }

    return null;
}
