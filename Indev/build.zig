const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.option(
        std.builtin.OptimizeMode,
        "optimize",
        "Optimization mode",
    ) orelse .ReleaseSafe;
    const options = b.addOptions();

    const libs = b.createModule(.{
        .root_source_file = b.path("libs/zig/libs.zig"),
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

    const exe = b.addExecutable(.{
        .name = "vm",
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

    exe.root_module.addImport("libs", libs);
    exe.root_module.addImport("state", state);
    exe.root_module.addImport("escapes", escapes);
    exe.root_module.addImport("tests", tests);
    exe.root_module.addImport("docs", docs);
    exe.root_module.addImport("allocator", allocator);
    
    libs.addImport("state", state);

    docs.addImport("allocator", allocator);

    escapes.addImport("libs", libs);
    
    options.addOption(bool, "VMDEBUG", optimize == .Debug);

    exe.root_module.addOptions("build_options", options);

    b.getInstallStep().dependOn(&install.step);
}
