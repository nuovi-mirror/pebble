const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

const timespec = extern struct {
    tv_sec: isize,
    tv_nsec: isize,
};

extern fn nanosleep(
    req: *const timespec,
    rem: ?*timespec,
) callconv(.c) c_int;

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.misc.sleep_ARG0") orelse return;
    const time = vm.data.get(
        try vm.valueToString(
            mem.temp(),
            indirect,
        ),
    ) orelse return;
    const timei = time.word; // do NOT pass this a floating-point value

    const ts = timespec{
        .tv_sec = @intCast(timei / 1000),
        .tv_nsec = @intCast((timei % 1000) * 1_000_000),
    };

    //std.debug.print("s: {}, ns: {}", .{ ts.tv_sec, ts.tv_nsec });
    _ = nanosleep(&ts, null);
}
