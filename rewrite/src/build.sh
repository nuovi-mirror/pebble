#!/bin/sh

findcc() {
	echo "Searching for a C compiler..."

	if [ -n "$1" ] && command -v "$1" && [ "$1" != "N" ]; then
		echo "Compiler given as argument three ($1). Using that."
	else
		if command -v cc >/dev/null; then
			CC="$(command -v cc)"
		else
			if command -v clang >/dev/null; then
				CC="$(command -v clang)"
			else
				if command -v gcc >/dev/null; then
					CC="$(command -v gcc)"
				else
					if command -v pcc >/dev/null; then
						CC="$(command -v pcc)"
					else
						echo "Error: No suitable C compiler found!"
						exit 1
					fi
				fi
			fi
		fi
	fi

	echo "C compiler found at $CC"
}

getplatform() {
	echo "Setting the platform..."
	
	if [ "$1" = "" ]; then
		echo "Error: No Platform Selected!"
		echo "Argument two should be the platform!"
		exit 1
	fi

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

	else
		echo "Error: Invalid Platform!"
		exit 1
	fi
}

findanyz() {
	echo "Searching for a static analyzer..."

	if [ -n "$1" ] && command -v "$1" && [ "$1" != "N" ]; then
		echo "Static analyzer selected at argument four ($1). Using that."
		ANYZ="$(command -v $1)"
	else
		if command -v scan-build >/dev/null; then
			ANYZ="$(command -v scan-build)"
		else
			if command -v scan-build-19 >/dev/null; then
				ANYZ="$(command -v scan-build-19)"
			else
				echo "Warning: No static analyzer found!"
			fi
		fi
	fi

	echo "Static analyzer found at $ANYZ"
}


getflags() {
	FLAGS="-std=c99"

	if [ "$1" = "default" ]; then FLAGS="$FLAGS -O2"
	elif [ "$1" = "fast" ]; then FLAGS="$FLAGS -O3 -flto -fno-semantic-interposition -ffast-math -march=native -mtune=native"
	elif [ "$1" = "debug" ]; then FLAGS="-g -O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wundef -Wcast-qual -Wcast-align -Wold-style-definition -Wswitch-enum -Wvla -Wdouble-promotion -Wfloat-equal -fsanitize=address,undefined -fno-omit-frame-pointer"
	else
		echo "Error: Build Type Not Selected!"
		echo "Argument one should be the platform!"
		exit 1
	fi

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

echo "Building test suite : $ANYZ $CC $FLAGS -o bim/test vm/test.c"
$ANYZ $CC $FLAGS -o bin/test vm/test.c

rm -rf platform/use/
rm vm/main.c

echo "Done!"
