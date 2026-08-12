const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.option(
        std.builtin.OptimizeMode,
        "optimize",
        "Optimization mode",
    ) orelse .ReleaseFast;

    const platform = b.createModule(.{
        .root_source_file = b.path(switch (target.result.os.tag) {
           .windows => "../../platform/win32.zig",
           else => "../../platform/posix.zig", // assume Windows if not POSIX
        }),
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "rockc-bootstrap",
        .root_module = b.createModule(.{
            .root_source_file = b.path("rock.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    const state = b.createModule(.{
        .root_source_file = b.path("../../src/state.zig"),
        .target = target,
        .optimize = optimize,
    });

    const install = b.addInstallArtifact(exe, .{
        .dest_dir = .{
            .override = .{
                .custom = "../",
            },
        },
    });

    exe.root_module.addAnonymousImport("stdlib", .{
        .root_source_file = b.path("../stdlib.rock"),
    });

    exe.root_module.addImport("platform", platform);
    exe.root_module.addImport("state", state);
    b.getInstallStep().dependOn(&install.step);
}
