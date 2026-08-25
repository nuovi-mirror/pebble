#!/bin/sh

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

echo "Setting platform libraries..."
rm platform/use/*
rm vm/main.c

if [ "$2" == "" ]; then
	echo "Error: No platform selected."
	exit 1
fi

# common
for i in platform/common/*; do
	echo "Linking platform/common/$(basename $i) to platform/use/"
	ln -s  "../common/$(basename $i)" platform/use/
done

# platform
for i in platform/$2/*; do
	echo "Linking platform/$2/$(basename $i) to platform/use/"
	ln -s "../$2/$(basename $i)" platform/use/
done

# entry point
echo "Linking platform entry point..."
ln -s ../platform/$2/main.c vm/main.c
rm platform/use/main.c


FLAGS="-std=c99"

SRCPATH="." # path to source directory
BINPATH="bin" # path to output directory

VMSRCNAME="vm/main.c" # VM source code file name
VMTESTSRCNAME="vm/test.c" # VMtest source code file name

VMSRCPATH="$SRCPATH/$VMSRCNAME"
VMTESTSRCPATH="$SRCPATH/$VMTESTSRCNAME"

VMBINNAME="vm" # VM output file name
VMTESTBINNAME="test" # VMtest output file name

VMBINPATH="$BINPATH/$VMBINNAME"
VMTESTBINPATH="$BINPATH/$VMTESTBINNAME"

if [ "$1" == "debug" ]; then
	#FLAGS="$FLAGS -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer"
	FLAGS="$FLAGS -g -O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wundef -Wcast-qual -Wcast-align -Wold-style-definition -Wswitch-enum -Wvla -Wdouble-promotion -Wfloat-equal"
elif [ "$1" == "fast" ]; then
	FLAGS="$FLAGS -O3 -flto -fno-semantic-interposition -ffast-math -march=native -mtune=native"
elif [ "$1" == "default" ]; then
	FLAGS="$FLAGS -O2"
else 
	echo "Error: Build type not found"
	exit 1
fi

mkdir -p "$BINPATH"

echo "Building VM: $CC "$FLAGS" -o $VMBINPATH $VMSRCPATH"
$CC $FLAGS -o $VMBINPATH $VMSRCPATH

echo "Building test suite: $CC "$FLAGS" -o $VMTESTBINPATH $VMTESTSRCPATH"
$CC $FLAGS -o $VMTESTBINPATH $VMTESTSRCPATH

rm platform/use/*
rm vm/main.c
echo "Done!"
