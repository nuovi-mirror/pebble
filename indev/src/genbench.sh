#!/bin/sh

start=2
max=400000

echo "New num '$start'"

for i in $(seq $start $max); do
	echo 'New num "num + num"'
done
