const std = @import("std");

const timespec = extern struct {
    tv_sec: isize,
    tv_nsec: isize,
};

extern fn nanosleep(
    req: *const timespec,
    rem: ?*timespec,
) callconv(.c) c_int;

pub fn run() !void {
    const timei = try std.fmt.parseInt(u32, "1000", 10);

    const ts = timespec{
        .tv_sec = @intCast(timei / 1000),
        .tv_nsec = @intCast((timei % 1000) * 1_000_000),
    };

    _ = nanosleep(&ts, null);
}
