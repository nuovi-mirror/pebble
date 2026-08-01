const std = @import("std");
const state = @import("state.zig");

const sndio = state.sndio;
// deinit audio device

pub fn run() !void {
    if (state.dev) |d| {
        _ = sndio.sio_flush(d);
        sndio.sio_close(d);
        state.dev = null;
    }
}
