#!/bin/sh

i=1
m=999999999

while [ $i -le $m ]; do
#	echo "Escape Random"
#	echo "New $i random"

	echo "New $i \"$i * $i\""

#	echo "New $i \"$i + 1\""
	
	i=$((i+1))
done
