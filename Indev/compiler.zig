const std = @import("std");

const allocator = std.heap.page_allocator;

pub fn main() !void {
    var args = std.process.args();
    _ = args.next(); // Skip program name
    const first_arg = args.next() orelse return;

    const data = try std.fs.cwd().readFileAlloc(
        allocator,
        first_arg,
        512 * 1024 * 1024,
    );
    defer allocator.free(data);

    var lines = std.mem.splitScalar(u8, data, '\n');
    var pending: []const u8 = "none";

    while (lines.next()) |line| {
        if (line.len == 0) continue;

        var tokens: std.ArrayList([]const u8) = .empty;
        defer tokens.deinit(allocator);

        var i: usize = 0;

        while (i < line.len) {
            // Skip whitespace
            while (i < line.len and std.ascii.isWhitespace(line[i])) {
                i += 1;
            }

            if (i >= line.len)
                break;

            // Start of token
            const start = i;

            // Consume token
            while (i < line.len and
                !std.ascii.isWhitespace(line[i]))
            {
                i += 1;
            }

            try tokens.append(allocator, line[start..i]);
        }

        if (tokens.items.len == 0)
            continue;

    
        var output: []u8 = undefined;

        if (std.mem.eql(u8, tokens.items[0], "fn")) { // fn funcname (funcargs) funcreturn {
                                                      // funcommands 
                                                      // }
            // Function definition
            if (tokens.items.len >= 2) {
                var _args: std.ArrayList([]const u8) = .empty;
                var inside = false;

                for (tokens.items) |token| {
                    if (std.mem.eql(u8, token, "(")) {
                        inside = true;
                        continue;
                    }

                    if (std.mem.eql(u8, token, ")")) {
                        break;
                    }

                    if (inside) {
                        try _args.append(allocator, token);
                    }
                }

                var buffer: std.ArrayList(u8) = .empty;

                for (_args.items, 0..) |arg, item| {
                    const thing: []const u8 = try std.fmt.allocPrint(
                        allocator,
                        "New {s} ARG{}\n",
                        .{ arg, item },
                    );

                    try buffer.appendSlice(allocator, thing);
                }
                
                var return_value: []const u8 = "none";

                for (tokens.items, 0..) |token, z| {
                    if (std.mem.eql(u8, token, ")")) {
                        if (z + 1 < tokens.items.len) {
                            return_value = tokens.items[z + 1];
                        }
                        break;
                    }
                }
            
                pending = return_value;
                output = try std.fmt.allocPrint(
                    allocator,
                    "Func {s}\n{s}",
                    .{ tokens.items[1], buffer.items },
                );

            }
        } else if (tokens.items.len >= 2 and
            std.mem.eql(u8, tokens.items[0], "return"))
        {
            output = try std.fmt.allocPrint(
                allocator,
                "New return '{s}'\nReturn",
                .{ tokens.items[1] },
            );
        } else if (tokens.items.len == 1 and
            std.mem.eql(u8, tokens.items[0], "}"))
        {
            output = try std.fmt.allocPrint(
                allocator,
                "\nNew return {s}\nEnd\n",
                .{ pending },
            );
        } else if (tokens.items.len >= 2 and
            std.mem.eql(u8, tokens.items[0], "call"))
        {
            var _args: std.ArrayList([]const u8) = .empty;
            var inside = false;

            for (tokens.items) |token| {
                if (std.mem.eql(u8, token, "(")) {
                    inside = true;
                    continue;
                }

                if (std.mem.eql(u8, token, ")")) {
                    break;
                }

                if (inside) {
                    try _args.append(allocator, token);
                }
            }

            var buffer: std.ArrayList(u8) = .empty;

            for (_args.items, 0..) |arg, item| {
                const thing: []const u8 = try std.fmt.allocPrint(
                    allocator,
                    "New ARG{} '{s}'\n",
                    .{ item, arg },
                );

                try buffer.appendSlice(allocator, thing);
            }

            output = try std.fmt.allocPrint(
                allocator, 
                "{s}Call {s}\n",
                .{ buffer.items, tokens.items[1] },
            );
        } else if (tokens.items.len >= 3 and
            std.mem.eql(u8, tokens.items[1], "="))
        {
            // Variable definition
            const omit = tokens.items[2][0];
            const first = tokens.items[2][1..];

            var value_tokens = tokens.items[2..];
            value_tokens[0] = first;

            const rest = try std.mem.join(
                allocator, 
                " ",
                value_tokens,
            );

            if (omit == '`') {
                // expression
                output = try std.fmt.allocPrint(
                    allocator,
                    "New {s} \"{s}\"",
                    .{ tokens.items[0], rest }
                );
            }
        } else if (std.mem.eql(u8, tokens.items[0], ".")) {
            const rest = try std.mem.join(
                allocator, 
                " ",
                tokens.items[1..],
            );
            output = rest;
        }
        std.debug.print("{s}\n", .{output}); // just print for now
    }
}
