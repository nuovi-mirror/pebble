const std = @import("std");
const allocator = std.heap.page_allocator;
const vm = @import("state");
const c = @cImport({@cInclude("time.h");}); // This code needs a POSIX libc

pub fn run() !void {
    var now: c.time_t = c.time(null);

    var tm: c.struct_tm = undefined;
    _ = c.localtime_r(&now, &tm);

    var buf: [64]u8 = undefined;
    const len = c.strftime(
        &buf[0],
        buf.len,
        "%a %b %d %H:%M:%S %Z %Y",
        &tm,
    );

    const copy = try allocator.dupe(u8, buf[0..len]);
    try vm.data.put("time", copy);
}
