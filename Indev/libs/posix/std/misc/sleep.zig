const std = @import("std");
const vm = @import("state");

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
    const time = vm.data.get(indirect) orelse return;
    const timei = try std.fmt.parseInt(u32, time, 10);

    const ts = timespec{
        .tv_sec = @intCast(timei / 1000),
        .tv_nsec = @intCast((timei % 1000) * 1_000_000),
    };

    _ = nanosleep(&ts, null);
}
