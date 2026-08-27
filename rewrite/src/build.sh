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

if [ "$2" == "" ]; then
	echo "Error: No platform selected."
	exit 1
fi

rm vm/main.c
mkdir platform/use/
echo "Linking $2 libraries..."

if [ "$2" == "freestanding" ]; then
	for i in platform/freestand/*; do
		ln -sf  "../freestand/$(basename $i)" platform/use/
	done
	
	ln -sf "../platform/freestand/main.c" vm/main.c

elif [ "$2" == "c" ]; then
	for i in platform/freestand/*; do
		ln -sf  "../freestand/$(basename $i)" platform/use/
	done

	for i in platform/c/*; do
		ln -sf  "../c/$(basename $i)" platform/use/
	done

	ln -sf "../platform/c/main.c" vm/main.c

elif [ "$2" = "openbsd" ]; then
	for i in platform/freestand/*; do
		ln -sf  "../freestand/$(basename $i)" platform/use/
	done

	for i in platform/c/*; do
		ln -sf  "../c/$(basename $i)" platform/use/
	done

	for i in platform/posix/*; do
		ln -sf  "../posix/$(basename $i)" platform/use/
	done

	for i in platform/$2/*; do
		ln -sf  "../$2/$(basename $i)" platform/use/
	done

	ln -sf "../platform/$2/main.c" vm/main.c

else
	for i in platform/freestand/*; do
		ln -sf  "../freestand/$(basename $i)" platform/use/
	done

	for i in platform/c/*; do
		ln -sf  "../c/$(basename $i)" platform/use/
	done

	for i in platform/$2/*; do
		ln -sf  "../$2/$(basename $i)" platform/use/
	done

	ln -sf "../platform/$2/main.c" vm/main.c
fi

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
rm -rf platform/use/
rm vm/main.c
echo "Done!"
