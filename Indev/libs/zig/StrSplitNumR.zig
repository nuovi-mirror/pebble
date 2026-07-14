const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("ARG1") orelse return;
    const str = vm.data.get(indirect) orelse return;
    const indirect2 = vm.data.get("ARG2") orelse return;
    const strNum = vm.data.get(indirect2) orelse return;
    const newStrNum = try std.fmt.parseInt(usize, strNum, 10);
    const newStr = str[newStrNum..];
    try vm.data.put("strRight", newStr);
}
