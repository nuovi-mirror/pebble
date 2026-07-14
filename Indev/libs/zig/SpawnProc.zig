const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator for now

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return error.VariableNotFound;
    const procArgs = vm.data.get(indirect) orelse return error.VariableNotFound;
   
    var argv = std.array_list.Managed([]const u8).init(allocator); 
    defer argv.deinit();

    var split = std.mem.splitScalar(u8, procArgs, ' ');
    while (split.next()) |arg| {
        try argv.append(arg);
    }

    var child = std.process.Child.init(
        argv.items,
        allocator,
    );

    try child.spawn();

    const pid = child.id;

    const pidAsString = try std.fmt.allocPrint(
        allocator,
        "{}",
        .{pid},
    );

    try vm.data.put("pid", pidAsString);
}
