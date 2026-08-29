<p>
    <img height=200px width=200px src="http://thenuoviorizzonticompany.org/logo.png"></img>
    CPebble
    	CPebble is an experiment at a modern and de-cluttered re-write of
    	Pebble in C. At some point this will be re-named to Pebble once
    	it is deemed stable enough to replace the current Zig version.
    	As the ZIg version, the licence can be viewed <a href="http://thenuoviorizzonticompany.org/license/openlicense/3.txt">here</a>.

    Building from source
    	Run the build.sh script using a POSIX shell
    	Argument one should be the build type and argument two should
    	be the target platform. The build types include fast as an
    	optimized build, which should not be ran on any machine besides
    	the build host, debug being the debugging build, and default
    	being the default production release option.
</p>

<p>
    Swapping targtes
    	Syslink or copy the files from your platform into platform/use.
    	Eg. if i am on a POSIX platform and want printing, I may do
    	
    		$ ln -s ../posix/print.h platform/use/print.h 
	
    	from the source root dir.

    	Then simply link the platform's main.c into the VM directory
    	This is normally handled by the build script.
</p>

<p>
    Platform layers
    	a platform layer must provide two things
    	  - a platform library
    	  - a platform entry
	
    	the platform library must impliment 
    	  - cmpstr (common)
    	  - copystr (common)
    	  - findnewline (common)
    	  - getstrlen (common)
    	  - skipspace (common)
    	  - exitproc (cimmon)
    	  - lalloc (common)
    	  - lcalloc (common)
    	  - lfree (common)
    	  - lrsize (common)
    	  - readfile
    	  - print
	
    	most of this is implimented by
    	the common platform-independent
    	libraries 
    	all of the ones not marked common
    	must be implimented by the platform
    	layer
	
    	the entry point must
    	  - define the NULL value (usually just with 
    		#define NULL ((void *)0) to be compliant with 2018 C)
    	  - set up a stack
    	  - get arguments
    	  - provide main()
    	  - call the main VM entry point (vmmain())
</p>

<p>
    Product of <a href="http://thenuoviorizzonticompany.org">The Nuovi Orizzonti Company</a>
</p>
