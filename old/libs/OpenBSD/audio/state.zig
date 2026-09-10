pub const sndio = @cImport({
        @cInclude("sndio.h");
});

pub var dev: ?*sndio.struct_sio_hdl = null;
// used to share state between things
