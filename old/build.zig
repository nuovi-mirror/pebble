const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.option(
        std.builtin.OptimizeMode,
        "optimize",
        "Optimization mode",
    ) orelse .ReleaseFast;

    const double = b.option(
        bool,
        "double",
        "Allow 64-bit width types",
    ) orelse false;

    const native = b.option(
        bool,
        "native",
        "Set VM width to host width",
    ) orelse false;

    const libs = b.createModule(.{
        .root_source_file = b.path(switch (target.result.os.tag) {
            .windows => "libs/win32/libs.zig",
            .openbsd => "libs/OpenBSD/libs.zig",
            else => "libs/posix/libs.zig",
        }),
        .target = target,
        .optimize = optimize,
        .link_libc = true, // sometimes not needed, better safe than sorry
    });

    const libs_general = b.createModule(.{
        .root_source_file = b.path(switch (target.result.os.tag) {
            .windows => "libs/win32/libs.zig",
            else => "libs/posix/libs.zig",
        }),
        .target = target,
        .optimize = optimize,
    });

    const state = b.createModule(.{
        .root_source_file = b.path("src/state.zig"),
        .target = target,
        .optimize = optimize,
    });

    const tests = b.createModule(.{
        .root_source_file = b.path("src/tests.zig"),
        .target = target,
        .optimize = optimize,
    });

    const escapes = b.createModule(.{
        .root_source_file = b.path("src/escapes.zig"),
        .target = target,
        .optimize = optimize,
    });

    const docs = b.createModule(.{
        .root_source_file = b.path("src/docs.zig"),
        .target = target,
        .optimize = optimize,
    });

    const allocator = b.createModule(.{
        .root_source_file = b.path("src/allocator.zig"),
        .target = target,
        .optimize = optimize,
    });

    const limits = b.createModule(.{
        .root_source_file = b.path("src/limits.zig"),
        .target = target,
        .optimize = optimize,
    });

    const version = b.createModule(.{
        .root_source_file = b.path("src/version.zig"),
        .target = target,
        .optimize = optimize,
    });

    const psh = b.createModule(.{
        .root_source_file = b.path("src/embedded/psh.pebble"),
        .target = target,
        .optimize = optimize,
    });

    const platform = b.createModule(.{
        .root_source_file = b.path(switch (target.result.os.tag) {
           .windows => "platform/win32.zig",
           else => "platform/posix.zig", // assume POSIX if not Windows
        }),
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "pblvm",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/vm.zig"),
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

    const vmoptions = b.addOptions();
    vmoptions.addOption(bool, "native", native);
    vmoptions.addOption(bool, "double", double);
    vmoptions.addOption(bool, "debug", optimize == .Debug);

    const build_options = vmoptions.createModule();

    allocator.addImport("build_options", build_options);

    state.addImport("build_options", build_options);
    state.addImport("allocator", allocator);

    exe.root_module.addImport("build_options", build_options);
    exe.root_module.addImport("libs", libs);
    exe.root_module.addImport("state", state);
    exe.root_module.addImport("escapes", escapes);
    exe.root_module.addImport("tests", tests);
    exe.root_module.addImport("docs", docs);
    exe.root_module.addImport("allocator", allocator);
    exe.root_module.addImport("limits", limits);
    exe.root_module.addImport("version", version);
    exe.root_module.addImport("psh", psh);
    exe.root_module.addImport("platform", platform);

    libs.addImport("state", state);
    libs.addImport("allocator", allocator);
    libs.addImport("general", libs_general);

    libs_general.addImport("state", state);
    libs_general.addImport("allocator", allocator);
    
    escapes.addImport("libs", libs);
    escapes.addImport("platform", platform);
    
    version.addImport("state", state);
    version.addImport("build_options", build_options);

    if (target.result.os.tag == .openbsd)
        exe.linkSystemLibrary("sndio");

    b.getInstallStep().dependOn(&install.step);
}
