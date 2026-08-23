#!/bin/sh

if [ "$1" == "d" ]; then
	cc -g -o pblvm vm/main.c
else
	cc -o pblvm vm/main.c
fi
