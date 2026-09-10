#!/bin/sh

cmd="./bootstrap/rockc-bootstrap" # path to rockc compiler
vm="../pblvm" # path to Pebble binary


"$cmd" "$1" >/tmp/tmp.pebble
"$vm" /tmp/tmp.pebble
