CC = cc
PROGRAM = bin/vm
BUILD = default
TARGET = c
ANALYZER =

CFLAGS = -std=c99
CPPFLAGS = \
	   -Ih \
	   -Iallocator \
	   -Istate \
	   -Ivm

# - build types - 
#   standard build 
BUILD_CFLAGS_default = \
		       -O2

#   debugging builds
BUILD_CFLAGS_debug = \
		     -g \
		     -O0 \
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
		    -flto \
		    -fno-semantic-interposition \
		    -ffast-math \
		    -march=native \
		    -mtune=native

CFLAGS += $(BUILD_CFLAGS_$(BUILD))

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
			    -Iplatform/posix \
			    -Iplatform/c \
			    -Iplatform/freestand

#   PureDarwin platform - needs a Darwin LIBC and Kernel
#   and the POSIX platform
#   and the C platform
#   and the freestand platform
PLATFORM_CPPFLAGS_puredarwin = \
			       -Iplatform/puredarwin \
			       -Iplatform/posix \
			       -Iplatform/c \
			       -Iplatform/freestand

CPPFLAGS += $(PLATFORM_CPPFLAGS_$(TARGET))

#  - VM sources -
VM_SRCS = \
	  allocator/allocator.c \
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
	 platform/c/lalloc.c \
	 platform/c/lcalloc.c \
	 platform/c/lfree.c \
	 platform/c/lrsize.c \
	 platform/c/setmem.c \
	 platform/c/print.c \
	 platform/c/readfile.c \
	 platform/c/snprint.c

#  - POSIX platform sources -
POSIX_SRCS = \
	     platform/posix/print.c \
	     platform/posix/readfile.c

# platform inheritance logic
PLATFORM_SRCS_freestand = $(FREESTAND_SRCS)
PLATFORM_SRCS_c = $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_posix = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_openbsd = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)
PLATFORM_SRCS_puredarwin = $(POSIX_SRCS) $(C_SRCS) $(FREESTAND_SRCS)

# entry point
MAIN_freestand = platform/freestand/main.c
MAIN_c = platform/c/main.c
MAIN_posix = platform/posix/main.c
MAIN_openbsd = platform/openbsd/main.c
MAIN_puredarwin = platform/puredarwin/main.c

MAIN = $(MAIN_$(TARGET))

SRCS = $(VM_SRCS) $(PLATFORM_SRCS_$(TARGET)) $(MAIN)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

# build the thing
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	mkdir bin
	$(ANALYZER) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

.c.o:
	$(ANALYZER) $(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(DEPS)

# clean up the thing
clean:
	rm -rf bin
	rm -f $(OBJS) $(DEPS) $(PROGRAM)

.PHONY: all clean
