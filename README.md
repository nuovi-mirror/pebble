Pebble is a small and simple virtual machine language
written in Zig.

The language is designed to be small, simple, reliable and safe, 
compiling down to one binary with no deps besides a libc and
dynamic loader, including a VM, compiler, interpreter, test suite,
embedded documentation and version info, all in less than 100kb.

Currently, there is effort to re-write the virtual machine in the
C language, hopefully to replace the current Zig version. This was
done because of how many unneeded layers exist during execution, due
to how hastily instruction pre-compiling was added.

Source code for the C re-write can be found in the rewrite/src/
directory from the root of this repository.

A list of mirrors is available <a href="http://pebblevm.org/mirrors.html">here</a>
The canonical source can be fetched <a href="http://pebblevm.org/git">here</a> over Git

Please note this item is still in an early Alpha phase.

Product of <a href="http://thenuoviorizzonticompany.org">The Nuovi Orizzonti Company</a>
