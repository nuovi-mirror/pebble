pub const vm = @import("state"); // VM state

pub const Print = @import("Print.zig"); // io
pub const PrintLn = @import("PrintLn.zig"); // io
//pub const Input = @import("Input.zig"); // io - POSIX

pub const Random = @import("Random.zig"); // misc
//pub const Time = @import("Time.zig"); // misc - POSIX

//pub const Fread = @import("Fread.zig"); // fs - POSIX 
//pub const Fwrite = @import("Fwrite.zig"); // fs - POSIX 
//pub const Fbwrite = @import("Fbwrite.zig"); // fs - POSIX
//pub const ListDir = @import("ListDir.zig"); // fs - POSIX
//pub const Gcwd = @import("Gcwd.zig"); // fs - POSIX
//pub const Chcwd = @import("Chcwd.zig"); // fs - POSIX

pub const StrLen = @import("StrLen.zig"); // str
pub const StrSplitNumL = @import("StrSplitNumL.zig"); // str
pub const StrSplitNumR = @import("StrSplitNumR.zig"); // str
pub const StrFind = @import("StrFind.zig"); // str

//pub const SpawnProc = @import("SpawnProc.zig"); // proc - POSIX
//pub const WaitPid = @import("WaitPid.zig"); // proc - POSIX
//pub const Exit = @import("Exit.zig"); // proc - POSIX
