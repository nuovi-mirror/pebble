const std = @import("std");
const vm = @import("state");

pub fn run() !void {
    const indirect = vm.data.get("__Escape_std.str.split.num.left_ARG0") 
        orelse return;
    const str = vm.data.get(indirect) orelse return;

    const indirect2 = vm.data.get("__Escape_std.str.split.num.left_ARG0") 
        orelse return;
    
    const strNum = vm.data.get(indirect2) orelse return;
    const newStrNum = try std.fmt.parseInt(usize, strNum, 10);
    const newStr = str[0..newStrNum];
    try vm.data.put("__Escape_std.str.split.num.left_RET0", newStr);
}
