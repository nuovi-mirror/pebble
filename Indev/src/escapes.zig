const std = @import("std");
const libs = @import("libs");

pub const escapeFn = *const fn () anyerror!void;

pub const Escape = struct {
    name: []const u8,
    run: escapeFn,
};

fn isNamespace(comptime T: type) bool {
    return switch (@typeInfo(T)) {
        .@"struct" => true,
        else => false,
    };
}

fn count(comptime T: type) usize {
    var total: usize = 0;

    inline for (@typeInfo(T).@"struct".decls) |decl| {
        const Child = @field(T, decl.name);

        // Any module with run() is an escape
        if (@hasDecl(Child, "run")) {
            total += 1;
            continue;
        }

        // Otherwise recurse into namespaces
        if (isNamespace(Child)) {
            total += count(Child);
        }
    }

    return total;
}

fn fill(
    comptime prefix: []const u8,
    comptime T: type,
    entries: []Escape,
    index: *usize,
) void {
    inline for (@typeInfo(T).@"struct".decls) |decl| {
        const Child = @field(T, decl.name);

        const name =
            if (prefix.len == 0)
                decl.name
            else
                std.fmt.comptimePrint("{s}.{s}", .{
                    prefix,
                    decl.name,
                });

        // Leaf: escape implementation
        if (@hasDecl(Child, "run")) {
            entries[index.*] = .{
                .name = name,
                .run = Child.run,
            };

            index.* += 1;
            continue;
        }

        // Branch: namespace
        if (isNamespace(Child)) {
            fill(name, Child, entries, index);
        }
    }
}

pub const table = blk: {
    @setEvalBranchQuota(1_000_000);

    var result: [count(libs)]Escape = undefined;
    var index: usize = 0;

    fill("", libs, &result, &index);

    break :blk result;
};

pub fn get(name: []const u8) ?Escape {
    for (table) |escape| {
        if (std.mem.eql(u8, escape.name, name)) {
            return escape;
        }
    }

    return null;
}

pub fn dump() void {
    for (table) |escape| {
        std.debug.print("{s}\n", .{escape.name});
    }
}
