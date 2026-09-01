#!/bin/sh

findcc() {
	echo "Searching for a C compiler..."

	if [ -n "$1" ] && command -v "$1" && [ "$1" != "N" ]; then
		if command -v cc >/dev/null; then
			CC="$(which cc)"
		else
			if command -v clang; then
				CC="$(which clang)"
			else 
				if command -v gcc; then
					CC="$(which gcc)"
				else
					if command -v pcc; then
						CC="$(which pcc)"
					else
						echo "Error: No suitable C compiler found!"
						exit 1
					fi
				fi
			fi
		fi
	fi
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
		if command -v scan-build-19 >/dev/null; then
			ANYZ="$(which scan-build-19)"
		else
			echo "Error: No suitable static analyzer found!"
			echo "Continuing..."
		fi
	fi
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
}

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

echo "Done!"
