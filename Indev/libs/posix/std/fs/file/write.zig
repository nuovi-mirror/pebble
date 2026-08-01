const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.fs.file.write_ARG0") orelse return;
    const fileData = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("__Escape_std.fs.file.write_ARG1") orelse return;
    const fileName = vm.data.get(indirect2) orelse return;
    
    const file = try std.fs.cwd().createFile(fileName, .{});
    defer file.close();

    const data = try parseEscapes(fileData);

    try file.writeAll(data);
}

fn parseEscapes(input: []const u8) ![]u8 {
    var output = std.ArrayList(u8){};
    defer output.deinit(mem.alloc());
    var i: usize = 0;

    while (i < input.len) {
        if (input[i] == '\\' and i + 1 < input.len) {
            switch (input[i]) {
                'n' => try output.append(mem.alloc(), '\n'),
                't' => try output.append(mem.alloc(), '\t'),
                'r' => try output.append(mem.alloc(), '\r'),
                '\\' => try output.append(mem.alloc(), '\\'),
                else => {
                    try output.append(mem.alloc(), '\\');
                    try output.append(mem.alloc(), input[i]);
                },
            }
        } else {
            try output.append(mem.alloc(), input[i]);
        }

        i += 1;
    }

    return output.toOwnedSlice(mem.alloc());
}
