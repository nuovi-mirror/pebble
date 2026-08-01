const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

fn xorshift(state: u16) u16 {
    var x = state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    return x;
}

pub fn run() !void {
    var seed: u16 = undefined;

    if (vm.data.get("__Escape_std.misc.random_last")) |last| {
        seed = try std.fmt.parseInt(u16, last, 10);
    } else {
        try std.posix.getrandom(std.mem.asBytes(&seed));
    }

    const next = xorshift(seed);

    const str = try std.fmt.allocPrint(
        mem.alloc(),
        "{}",
        .{next},
    );

    try vm.data.put("__Escape_std.misc.random_last", str);
    try vm.data.put("__Escape_std.misc.random_RET0", str);
}
