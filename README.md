<a href="http://pebblevm.org">Pebble</a> is a small and simple virtual machine language
written in C.

[ NOTE ] - The old implimentation of Pebble, which was written in Zig, has become
depreciated. If, for some reason, you still wish to use this, it can be downloaded
at our main site <a href="http://pebblevm.org/downloads/old/">here</a>

Pebble is a highly portable, ultralight, and fully host-safe
virtual machine executing the Pebble bytecode language. Pebble
internally represents instructions as full tree-structure IRs,
preserving higher-level schematics, such as instruction, 
operation, and data relationships, allowing for optimization
inside of the virtual machine itself. Pebble also allows for
modifier addressing modes, which allow the execution engine
to change how it may view the operands of the instruction.
These range from things like simple pointers to full expression
tree evaluation by simply changing an addressing mode. These
addressing modes are applied mostly uniform across the virtual
machine, including for more complex instructions, such as those
for function definition. This allows for higher-level constructs,
such as advanced metaprogramming, to be represented without
special or dedicated instructions. Although Pebble is such a
small language, it has several advanced features, such as 
a host-safe architecture, full FFI, and as stated above,
advanced metaprogramming abilities.

Currently, there is effort to re-write the virtual machine in the
C language, hopefully to replace the current Zig version. This was
done because of how many unneeded layers exist during execution, due
to how hastily instruction pre-compiling was added.

CPebble has recently been moved to the indev/ directory. Please note
CPebble is not currently feature-complete and has yet to add all of
the Pebble schematics, unlike the older Zig version, which has been moved
to old/.

A list of mirrors is available <a href="http://pebblevm.org/mirrors.html">here</a>
The canonical source can be fetched <a href="http://pebblevm.org/git">here</a> over Git

Please note this item is still in an early Alpha phase.

Product of <a href="http://thenuoviorizzonticompany.org">The Nuovi Orizzonti Company</a>

