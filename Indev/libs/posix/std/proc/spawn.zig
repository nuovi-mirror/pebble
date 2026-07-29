const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return error.VariableNotFound;
    const procArgs = vm.data.get(indirect) orelse return error.VariableNotFound;
   
    var argv = std.array_list.Managed([]const u8).init(mem.alloc()); 
    defer argv.deinit();

    var split = std.mem.splitScalar(u8, procArgs, ' ');
    while (split.next()) |arg| {
        const expanded = vm.data.get(arg) orelse arg;
        try argv.append(expanded);
    }

    var child = std.process.Child.init(
        argv.items,
        mem.alloc(),
    );

    try child.spawn();

    const pid = child.id;

    const pidAsString = try std.fmt.allocPrint(
        mem.alloc(),
        "{}",
        .{pid},
    );

    try vm.data.put("pid", pidAsString);
}
