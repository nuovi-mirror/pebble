const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

fn xorshift(state: u32) u32 {
    var x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

pub fn run() !void {
    var seed: u32 = undefined;

    if (vm.data.get("random.last")) |last| {
        seed = try std.fmt.parseInt(u32, last, 10);
    } else {
        seed = std.crypto.random.int(u32);
    }

    const next = xorshift(seed);

    const str = try std.fmt.allocPrint(
        mem.alloc(),
        "{}",
        .{next},
    );

    try vm.data.put("random.last", str);
    try vm.data.put("random", str);
}
