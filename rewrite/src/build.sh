#!/bin/sh

FLAGS="-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wundef -Wcast-qual -Wcast-align -Wold-style-definition -Wswitch-enum -Wvla -Wdouble-promotion -Wfloat-equal"

echo "Finding C compiler..."
if command -v cc >/dev/null; then
	CC="$(which cc)"
else
	if command -v clang >/dev/null; then
		CC="$(which clang)"
	else
		if command -v pcc >/dev/null; then
			CC="$(which pcc)"
		else
			if command -v gcc >/dev/null; then
				CC="$(which gcc)"
			else
				echo "Error: No suitable C compiler found."
				exit 1
			fi
		fi
	fi
fi

echo "C compiler found at $CC"

SRCPATH="." # path to source directory
BINPATH="bin" # path to output directory

VMSRCNAME="vm/main.c" # VM source code file name
VMTESTSRCNAME="vm/test.c" # VMtest source code file name

VMSRCPATH="$SRCPATH/$VMSRCNAME"
VMTESTSRCPATH="$SRCPATH/$VMTESTSRCNAME"

VMBINNAME="pblvm" # VM output file name
VMTESTBINNAME="vmtest" # VMtest output file name

VMBINPATH="$BINPATH/$VMBINNAME"
VMTESTBINPATH="$BINPATH/$VMTESTBINNAME"

if [ "$1" == "d" ]; then
	FLAGS="$FLAGS -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer"
else
	FLAGS="$FLAGS -O3"
fi

mkdir -p "$BINPATH"

echo "Building VM: $CC "$FLAGS" -o $VMBINPATH $VMSRCPATH"
$CC $FLAGS -o $VMBINPATH $VMSRCPATH

echo "Building test suite: $CC "$FLAGS" -o $VMTESTBINPATH $VMTESTSRCPATH"
$CC $FLAGS -o $VMTESTBINPATH $VMTESTSRCPATH

echo "Done!"
