# variables starting with a lowercase char may be
# modified by the user

cc = cc
program = bin/vm
build = default
target = c
analyzer =

cflags =
ldflags =
CPPFLAGS = \
	   -Ih \
	   -Iffi

# - build types - 
#   standard build 
BUILD_CFLAGS_default = \
	       	     -O2 \
		     -static \
		     -flto \
		     -ffunction-sections \
		     -fdata-sections

#   debugging builds
BUILD_CFLAGS_debug = \
		     -g \
		     -O0 \
		     -static \
		     -Wall \
		     -Wextra \
		     -Wpedantic \
		     -Wshadow \
		     -Wconversion \
		     -Wsign-conversion \
		     -Wformat=2 \
		     -Wundef \
		     -Wcast-qual \
		     -Wcast-align \
		     -Wold-style-definition \
		     -Wswitch-enum \
		     -Wvla \
		     -Wdouble-promotion \
		     -Wfloat-equal

#   optimized builds
#   please note this may not work on
#   a machine that is not the machine
#   compiling - do NOT use this type
#   for cross-compiling or distributing
BUILD_CFLAGS_fast = \
		    -O3 \
		    -ffunction-sections \
		    -fdata-sections \
		    -static \
		    -flto \
		    -fno-semantic-interposition \
		    -ffast-math \
		    -march=native \
		    -mtune=native

#   smaller builds - not statically linked for size
BUILD_CFLAGS_small = \
		     -Oz \
		     -flto \
		     -ffunction-sections \
		     -fdata-sections

cflags += \
	  -std=c99 \
	  $(BUILD_CFLAGS_$(build))

ldflags += \
	   -Wl,--gc-sections

# - platform include paths -
#   freestanding platform - needs nothing but itself
PLATFORM_CPPFLAGS_freestand = \
			      -Iplatform/freestand

#   C platform - needs a hosted C99 LIBC
#   and the freestand platform
PLATFORM_CPPFLAGS_c = \
		      -Iplatform/c \
		      -Iplatform/freestand

#   OpenBSD platform - needs an OpenBSD Kernel
#   and the POSIX platform
#   and the C platform
#   and the freestand platform
PLATFORM_CPPFLAGS_openbsd = \
			    -Iplatform/openbsd \
			    -Iplatform/c \
			    -Iplatform/freestand

#   PureDarwin platform - needs a Darwin LIBC and Kernel
#   and the POSIX platform
#   and the C platform
#   and the freestand platform
PLATFORM_CPPFLAGS_puredarwin = \
			       -Iplatform/puredarwin \
			       -Iplatform/c \
			       -Iplatform/freestand

CPPFLAGS += $(PLATFORM_CPPFLAGS_$(target))

#  - VM sources -
VM_SRCS = \
	  allocator/allocator.c \
	  allocator/mem.c \
	  state/evaluator.c \
	  state/expressions.c \
	  state/functions.c \
	  state/hashmap.c \
	  state/instructionmapper.c \
	  state/instructions.c \
	  state/values.c \
	  state/variables.c \
	  vm/vm.c

#  - freestand platform sources -
FREESTAND_SRCS = \
		 platform/freestand/cmpstr.c \
		 platform/freestand/cmpstrn.c \
		 platform/freestand/copystr.c \
		 platform/freestand/findnewline.c \
		 platform/freestand/getnstrlen.c \
		 platform/freestand/getstrlen.c \
		 platform/freestand/skipspace.c \
		 platform/freestand/copymem.c \
		 platform/freestand/strsplit.c

#  - C platform sources -
C_SRCS = \
	 platform/c/entry.c \
	 platform/c/exitproc.c \
	 platform/c/setmem.c \
	 platform/c/print.c \
	 platform/c/readfile.c \
	 platform/c/str2ul.c \
	 platform/c/snprint.c

# main FFI sources
BASE_FFI_SRCS = \
	   ffi/ffi.c \
	   ffi/escapes.c

# FFI sources for the C platform
C_FFI_SRCS = \
	   ffi/platform/c/test/hello.c \
	   ffi/platform/c/std/io/print.c \
	   ffi/platform/c/std/io/printLn.c

# platform inheritance logic
PLATFORM_SRCS_freestand = $(FREESTAND_SRCS)
PLATFORM_SRCS_c = $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_posix = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_openbsd = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_puredarwin = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)

# FFI inheritance logic
FFI_SRCS_c = $(BASE_FFI_SRCS) $(C_FFI_SRCS)

# entry point
MAIN_freestand = platform/freestand/main.c
MAIN_c = platform/c/main.c
MAIN_posix = platform/posix/main.c
MAIN_openbsd = platform/openbsd/main.c
MAIN_puredarwin = platform/puredarwin/main.c

MAIN = $(MAIN_$(target))

SRCS = $(VM_SRCS) $(PLATFORM_SRCS_$(target)) $(FFI_SRCS_$(target)) $(MAIN)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

# build the thing
all: $(program)

$(program): $(OBJS)
	rm -rf bin
	mkdir bin
	$(analyzer) $(cc) $(cflags) $(ldflags) -o $@ $(OBJS)

.c.o:
	$(analyzer) $(cc) $(CPPFLAGS) $(cflags) -MMD -MP -c -o $@ $<

-include $(DEPS)

# clean up the thing
clean:
	rm -rf bin
	rm -f $(OBJS) $(DEPS) $(program)

.PHONY: all clean bear
