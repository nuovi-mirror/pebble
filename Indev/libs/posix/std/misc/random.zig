const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

fn xorshift(state: vm.word) vm.word {
    var x = state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    return x;
}

pub fn run() !void {
    var seed: vm.word = undefined;

    if (vm.data.get("__Escape_std.misc.random_last")) |last| {
        seed = last.word;
    } else {
        try std.posix.getrandom(std.mem.asBytes(&seed));
    }

    const next = xorshift(seed);

    try vm.data.put("__Escape_std.misc.random_last", .{ .word = next});
    try vm.data.put("__Escape_std.misc.random_RET0", .{ .word = next});
}
