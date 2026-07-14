pub const vm = @import("state"); // VM state

pub const Print = @import("Print.zig"); // io
pub const Input = @import("Input.zig"); // io - POSIX

pub const Random = @import("Random.zig"); // misc
pub const Time = @import("Time.zig"); // misc - POSIX

pub const Fread = @import("Fread.zig"); // fs
pub const Fwrite = @import("Fwrite.zig"); // fs
pub const Fbwrite = @import("Fbwrite.zig"); // fs

pub const StrLen = @import("StrLen.zig"); // str
pub const StrSplitNumL = @import("StrSplitNumL.zig"); // str
pub const StrSplitNumR = @import("StrSplitNumR.zig"); // str

pub const SpawnProc = @import("SpawnProc.zig"); // proc - POSIX
pub const WaitPid = @import("WaitPid.zig"); // proc - POSIX
