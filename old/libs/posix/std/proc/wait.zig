const std = @import("std");
const vm = @import("state");
const mem = @import("allocator");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.proc.wait_ARG0") 
        orelse return error.VariableNotFound;
    
    const pid = vm.data.get(indirect) orelse return error.VariableNotFound;

    const realPid = try std.fmt.parseInt(std.posix.pid_t, pid, 10);

    const result = std.posix.waitpid(realPid, 0);

    const status = result.status;

    // return -0 if does not appl

    if ((status & 0x7f) == 0) {
        // Normal exit
        const code = (status >> 8) & 0xff;

        const exitCodeAsString = try std.fmt.allocPrint(
            mem.alloc(),
            "{}",
            .{code},
        );

        try vm.data.put("__Escape_std.proc.wait_RET0", exitCodeAsString);
        try vm.data.put("__Escape_std.proc.wait_RET1", "-0");
        try vm.data.put("__Escape_std.proc.wait_RET2", "1");

    } else {
        // Signal termination
        const sig = status & 0x7f;

        const sigAsString = try std.fmt.allocPrint(
            mem.alloc(),
            "{}",
            .{sig},
        );

        try vm.data.put("__Escape_std.proc.wait_RET0", "-0");
        try vm.data.put("__Escape_std.proc.wait_RET1", sigAsString);
        try vm.data.put("__Escape_std.proc.wait_RET2", "0");
    }
}
