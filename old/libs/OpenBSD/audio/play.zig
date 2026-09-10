const std = @import("std");
const vm = @import("state");
const state = @import("state.zig");

const sndio = state.sndio;

// takes nice & simple
// - 32bit float
// - 48khz
// - sterio
// converts to whatever the backend uses
// plays

pub fn run() !void {
    const indirect = vm.data.get("__Escape_audio.play_ARG0") orelse return;
    const samplesFromTheVM = vm.data.get(indirect) orelse return;

    const samples = std.mem.bytesAsSlice(
        f32, 
        samplesFromTheVM,
    );


//    for (0..100_000_000_000_000) |i| {
//        if (samples[i] != 0) {
//            std.debug.print("{d}\n", .{samples[i]});
//            break;
//        }
//    }

//    for (0..32) |i| {
//        std.debug.print("{x:0>2} ", .{samples[i]});
//    }
//    std.debug.print("\n", .{});

//    for (0..32) |i| {
//        std.debug.print("{x:0>2} ", .{samplesFromTheVM[i]});
//    }
//    std.debug.print("\n", .{});

//    var max: f32 = 0;

//    for (samples) |s| {
//        if (@abs(s) > max)
//            max = @abs(s);
//    }
//    std.debug.print("max amplitude: {d}\n", .{max});

//    var first: usize = 0;
//    var found = false;

//    for (samples, 0..) |s, i| {
//        if (@abs(s) > 0.001) {
//            first = i;
//                found = true;
//                    break;
//        }
//    }

//    if (found) {
//       std.debug.print("first real sample: {} value {d}\n", .{
//            first,
//                samples[first],
//        });
//    }


    var pcm: [4096]i16 = undefined;

    var pos: usize = 0;

    while (pos < samples.len) {
        const amount = @min(
            pcm.len,
            samples.len - pos,
        );

        for (0..amount) |i| {
            const s = std.math.clamp(
                samples[pos + i],
                -1.0,
                1.0,
            );

            pcm[i] = @intFromFloat(
                s * 32767.0,
            );
        }

        const pcm_bytes = std.mem.sliceAsBytes(
            pcm[0..amount],
        );

//        if (sndio.sio_write(
//            state.dev.?,
//            &pcm,
//            bytes,
//        ) != bytes)
//            return error.WriteFailed;


        try writeAll(pcm_bytes);

        pos += amount;
    }
}

fn writeAll(buf: []const u8) !void {
    var offset: usize = 0;

    while (offset < buf.len) {
        const n = sndio.sio_write(
            state.dev.?,
            buf[offset..].ptr,
            buf.len - offset,
        );

        if (n == 0)
            return error.WriteFailed;

        offset += n;
    }
}
