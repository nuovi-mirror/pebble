#!/bin/sh

findcc() {
	echo "Searching for a C compiler..."

	[ -n "$1" ] && command -v "$1" && [ "$1" != "N" ] && CC="$(command -v  $1)" && return

	command -v cc    && CC="$(command -v cc)"    && return
	command -v gcc   && CC="$(command -v gcc)"   && return
	command -v clang && CC="$(command -v clang)" && return
	command -v pcc   && CC="$(command -v pcc)"   && return
		
	echo "Error: No suitable C compiler found!"
	exit 1

	echo "C compiler found at $CC"
}

getplatform() {
	echo "Setting the platform..."
	
	[ "$1" = "" ] && echo "Error: No Platform Selected!" && echo "Argument two should be the platform!" && exit 1

	rm -rf platform/use/ >/dev/null 2>&1
	mkdir platform/use/

	if [ "$1" = "freestand" ]; then
		for i in platform/freestand/*; do
			if [ -e "platform/use/$i" ]; then
				rm "platform/use/$i"
			fi

			ln -s "../freestand/$(basename $i)" platform/use/
		done

		ln -s "../platform/freestand/main.c" vm/main.c

	elif [ "$1" = "c" ]; then
		for i in platform/freestand/*; do
			if [ -e "platform/use/$i" ]; then
				rm "platform/use/$i"
			fi

			ln -s "../freestand/$(basename $i)" platform/use/
		done
		
		for i in platform/c/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../c/$(basename $i)" platform/use/
		done

		ln -s "../platform/c/main.c" vm/main.c

	elif [ "$1" = "posix" ]; then
		for i in platform/freestand/*; do
			if [ -e "platform/use/$i" ]; then
				rm "platform/use/$i"
			fi

			ln -s "../freestand/$(basename $i)" platform/use/
		done
		
		for i in platform/c/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../c/$(basename $i)" platform/use/
		done
		
		for i in platform/posix/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../posix/$(basename $i)" platform/use/
		done

		ln -s "../platform/posix/main.c" vm/main.c

	elif [ "$1" = "openbsd" ]; then
		for i in platform/freestand/*; do
			if [ -e "platform/use/$i" ]; then
				rm "platform/use/$i"
			fi

			ln -s "../freestand/$(basename $i)" platform/use/
		done
		
		for i in platform/c/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../c/$(basename $i)" platform/use/
		done
		
		for i in platform/posix/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../posix/$(basename $i)" platform/use/
		done

		for i in platform/openbsd/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../openbsd/$(basename $i)" platform/use/
		done

		ln -s "../platform/openbsd/main.c" vm/main.c

	elif [ "$1" = "puredarwin" ]; then
		for i in platform/freestand/*; do
			if [ -e "platform/use/$i" ]; then
				rm "platform/use/$i"
			fi

			ln -s "../freestand/$(basename $i)" platform/use/
		done
		
		for i in platform/c/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../c/$(basename $i)" platform/use/
		done
		
		for i in platform/posix/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../posix/$(basename $i)" platform/use/
		done

		for i in platform/puredarwin/*; do
			if [ -e "platform/use/$(basename $i)" ]; then
				rm "platform/use/$(basename $i)"
			fi

			ln -s "../puredarwin/$(basename $i)" platform/use/
		done

		ln -s "../platform/puredarwin/main.c" vm/main.c
	
	else
		echo "Error: Invalid Platform!"
		exit 1
	fi
}

findanyz() {
	echo "Searching for a static analyzer..."

	[ -n "$1" ] && command -v "$1" && [ "$1" != "N" ] && ANYZ="$(command -v $1)" && return

	command -v scan-build    && ANYZ="$(command -v scan-build)"    && return
	command -v scan-build-21 && ANYZ="$(command -v scan-build-21)" && return
	command -v scan-build-20 && ANYZ="$(command -v scan-build-20)" && return
	command -v scan-build-19 && ANYZ="$(command -v scan-build-19)" && return
	command -v scan-build-18 && ANYZ="$(command -v scan-build-18)" && return
	command -v scan-build-17 && ANYZ="$(command -v scan-build-17)" && return
	echo "Warning: No static analyzer found!"

	echo "Static analyzer found at $ANYZ"
}


getflags() {
	FLAGS="-std=c99"

	[ "$1" = "default" ] && FLAGS="$FLAGS -O2" && return
	[ "$1" = "fast" ] && FLAGS="$FLAGS -O3 -flto -fno-semantic-interposition -ffast-math -march=native -mtune=native" && return
	[ "$1" = "debug" ] && FLAGS="-g -O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wundef -Wcast-qual -Wcast-align -Wold-style-definition -Wswitch-enum -Wvla -Wdouble-promotion -Wfloat-equal -fno-omit-frame-pointer" && return

	echo "Error: Build Type Not Selected!"
	echo "Argument one should be the build type!"
	exit 1

	echo "Build type of $1"
	echo "Flags of: $FLAGS"
}

rm -r platform/use/ >/dev/null 2>&1
rm vm/main.c >/dev/null 2>&1

getflags $1
getplatform $2
findcc $3
findanyz $4

rm -rf bin/
mkdir -p bin/

echo "Building VM         : $ANYZ $CC $FLAGS -o bin/vm vm/main.c"
$ANYZ $CC $FLAGS -o bin/vm vm/main.c

echo "Building test suite : $ANYZ $CC $FLAGS -o bin/test vm/test.c"
$ANYZ $CC $FLAGS -o bin/test vm/test.c

#rm -rf platform/use/
#rm vm/main.c

echo "Done!"
