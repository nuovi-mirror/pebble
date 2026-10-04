#!/bin/sh

if [ "$1" = "clean" ]; then
	rm rockc
	rm demo.pebble
	rm stdlib.o

	exit 0
fi

cc -o stdlib.o -c stdlib.c
cc -o rockc rockc.c stdlib.o

chmod +x rockc

./rockc demo.newrock . >demo.pebble
