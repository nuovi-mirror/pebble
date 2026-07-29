const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return error.VariableNotFound;
    const pid = vm.data.get(indirect) orelse return error.VariableNotFound;

    const realPid = try std.fmt.parseInt(std.posix.pid_t, pid, 10);

    const result = std.posix.waitpid(realPid, 0);

    const status = result.status;

    if ((status & 0x7f) == 0) {
        // Normal exit
        const code = (status >> 8) & 0xff;

        const exitCodeAsString = try std.fmt.allocPrint(
            mem.alloc(),
            "{}",
            .{code},
        );

        try vm.data.put("exitCode", exitCodeAsString);
        try vm.data.put("otherTermination", "1");

    } else {
        // Signal termination
        const sig = status & 0x7f;

        const sigAsString = try std.fmt.allocPrint(
            mem.alloc(),
            "{}",
            .{sig},
        );

        try vm.data.put("killedBySig", sigAsString);
        try vm.data.put("otherTermination", "0");
    }
}
