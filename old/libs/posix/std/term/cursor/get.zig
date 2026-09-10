const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    var stdout_buf: [1024]u8 = undefined;
    var stdout_writer = std.fs.File.stdout().writer(&stdout_buf);
    const stdout = &stdout_writer.interface;

    var stdin_buf: [32]u8 = undefined;
    const stdin = std.fs.File.stdin();

    // enter raw mode
    const original = try std.posix.tcgetattr(stdin.handle);
    var raw = original;

    raw.lflag.ICANON = false;
    raw.lflag.ECHO = false;

    try std.posix.tcsetattr(
        stdin.handle,
        .NOW,
        raw,
    );

    try stdout.print("\x1b[6n", .{}); // ask for pos
    try stdout.flush();

    const amount = try stdin.read(&stdin_buf); // read stdin buffer
    const response = stdin_buf[0..amount]; // grab chars

    // Expected: 
    // ESC [ row ; col R

    if (response.len < 6)
        return error.InvalidResponse;

    if (response[0] != 0x1b or response[1] != '[')
        return error.InavlidResponse;

    var split = std.mem.splitScalar(u8, response[2..], ';');

    const y_str = split.next() orelse return error.InavlidResponse;
    var x_str = split.next() orelse return error.InavlidResponse;

    if (x_str[x_str.len - 1] != 'R')
        return error.InvalidResponse;

    x_str = x_str[0 .. x_str.len - 1]; // remove trailing R

    try vm.data.put("__Escape_std.term.cursor.get_RET0", x_str);
    try vm.data.put("__Escape_std.term.cursor.get_RET1", y_str);

    // leave raw mode
    try std.posix.tcsetattr(
        stdin.handle,
        .NOW,
        original,
    );
}
