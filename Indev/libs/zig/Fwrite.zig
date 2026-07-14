const std = @import("std");
const vm = @import("state");
const allocator = std.heap.page_allocator; // just use the OS allocator for now

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const fileData = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("ARG2") orelse return;
    const fileName = vm.data.get(indirect2) orelse return;
    
    const file = try std.fs.cwd().createFile(fileName, .{});
    defer file.close();

    const data = try parseEscapes(fileData);
    defer allocator.free(data);

    try file.writeAll(data);
}

fn parseEscapes(input: []const u8) ![]u8 {
    var output = std.ArrayList(u8){};
    defer output.deinit(allocator);
    var i: usize = 0;

    while (i < input.len) {
        if (input[i] == '\\' and i + 1 < input.len) {
            switch (input[i]) {
                'n' => try output.append(allocator, '\n'),
                't' => try output.append(allocator, '\t'),
                'r' => try output.append(allocator, '\r'),
                '\\' => try output.append(allocator, '\\'),
                else => {
                    try output.append(allocator, '\\');
                    try output.append(allocator, input[i]);
                },
            }
        } else {
            try output.append(allocator, input[i]);
        }

        i += 1;
    }

    return output.toOwnedSlice(allocator);
}
