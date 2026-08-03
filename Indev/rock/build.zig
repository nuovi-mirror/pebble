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
           .windows => "../platform/win32.zig",
           else => "../platform/posix.zig", // assume Windows if not POSIX
        }),
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "rockc",
        .root_module = b.createModule(.{
            .root_source_file = b.path("rock.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    const install = b.addInstallArtifact(exe, .{
        .dest_dir = .{
            .override = .{
                .custom = "../",
            },
        },
    });

    exe.root_module.addImport("platform", platform);
    b.getInstallStep().dependOn(&install.step);
}
