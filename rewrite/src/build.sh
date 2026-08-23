#!/bin/sh

if [ "$1" == "d" ]; then
	cc -g -o pblvm vm/main.c
	cc -o -g test vm/test.c
else
	cc -o pblvm vm/main.c
	cc -o test vm/test.c
fi
