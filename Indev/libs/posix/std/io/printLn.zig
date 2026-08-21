const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.io.print_ARG0") orelse return;
    const msg = try vm.valueToString(
        mem.temp(),
        vm.data.get(
            try vm.valueToString(
                mem.temp(),
                indirect,
            ),
        ) orelse return,
    );

    var buf: [1024]u8 = undefined;
    var writer = std.fs.File.stdout().writer(&buf);
    const stdout = &writer.interface;

    try stdout.print("{s}", .{msg});
    try stdout.flush();
}

