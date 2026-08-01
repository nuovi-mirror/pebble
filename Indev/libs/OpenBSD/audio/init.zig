const std = @import("std");
const state = @import("state.zig");

const sndio = state.sndio;
// init audio for playing

pub fn run() !void {
    state.dev = sndio.sio_open(
        sndio.SIO_DEVANY,
        sndio.SIO_PLAY,
        0,
    ) orelse return error.OpenFailed;

    var par: sndio.struct_sio_par = undefined;
    sndio.sio_initpar(&par);

    par.rate = 48000;
    par.pchan = 2;
    par.bits = 16;
    par.sig = 1;
    par.le = 1;

    if (sndio.sio_setpar(state.dev, &par) == 0)
        return error.BadFormat;

    if (sndio.sio_start(state.dev, ) == 0)
        return error.StartFailed;
}
