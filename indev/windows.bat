@echo off
setlocal

set CC=mingw32-gcc
set OUT=vm.exe

set SRCS=vm\vm.c ^
allocator\allocator.c ^
state\evaluator.c ^
state\expressions.c ^
state\functions.c ^
state\hashmap.c ^
state\instructionmapper.c ^
state\instructions.c ^
state\values.c ^
state\variables.c ^
platform\c\copymem.c ^
platform\c\entry.c ^
platform\c\exitproc.c ^
platform\c\lalloc.c ^
platform\c\lcalloc.c ^
platform\c\lfree.c ^
platform\c\lrsize.c ^
platform\c\setmem.c ^
platform\c\snprint.c ^
platform\c\readfile.c ^
platform\c\print.c ^
platform\c\main.c ^
platform\freestand\cmpstr.c ^
platform\freestand\cmpstrn.c ^
platform\freestand\copystr.c ^
platform\freestand\findnewline.c ^
platform\freestand\getnstrlen.c ^
platform\freestand\getstrlen.c ^
platform\freestand\skipspace.c ^
platform\freestand\strsplit.c


set INCLUDES=-Ivm -Iallocator -Istate -Iplatform\c

echo Building %OUT%...
%CC% -o %OUT% %SRCS% %INCLUDES%

if errorlevel 1 (
    echo.
    echo Build FAILED.
    exit /b 1
) else (
    echo.
    echo Build succeeded: %OUT%
)

endlocal
