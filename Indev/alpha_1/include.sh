#!/bin/sh

echo "# Automatic Include" >/tmp/tbdlang.tmp
echo "# Library from $2 pulled" >>/tmp/tbdlang.tmp
echo "# Version Alpha 1, Ruby" >>/tmp/tbdlang.tmp
cat "$2" >>/tmp/tbdlang.tmp
echo "# End Automatic Include" >>/tmp/tbdlang.tmp
cat "$1" >>/tmp/tbdlang.tmp
tbdlang-alpha_1-interpreter /tmp/tbdlang.tmp
