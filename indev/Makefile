CC?=cc
PROGRAM=bin/vm
BUILD?=default
TARGET?=posix
PLATFORM?=$(TARGET)
ANALYZER?=
CFLAGS=-std=c99
CPPFLAGS=

# build type
.if ${BUILD} == "default"
CFLAGS+=-O2

.elif ${BUILD} == "debug"
CFLAGS+=-g -O0 \
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

.elif ${BUILD} == "fast"
CFLAGS+=-O3 \
	-flto \
	-fno-semantic-interposition \
	-ffast-math \
	-march=native \
	-mtune=native

.else
.error Unknown build type: ${BUILD}
.endif

# common include paths
CPPFLAGS+=-Iallocator
CPPFLAGS+=-Istate
CPPFLAGS+=-Ivm

# platform
.if ${PLATFORM} == "freestand"
PLATFORM_DIRS=platform/freestand
MAIN=platform/freestand/main.c

.elif ${PLATFORM} == "c"
PLATFORM_DIRS=platform/c platform/freestand
MAIN=platform/c/main.c

.elif ${PLATFORM} == "posix"
PLATFORM_DIRS=platform/posix platform/c platform/freestand
MAIN=platform/posix/main.c

.elif ${PLATFORM} == "openbsd"
PLATFORM_DIRS=platform/openbsd platform/posix platform/c platform/freestand
MAIN=platform/openbsd/main.c

.elif ${PLATFORM} == "puredarwin"
PLATFORM_DIRS=platform/puredarwin platform/posix platform/c platform/freestand
MAIN=platform/puredarwin/main.c

.else
.error Unknown platform: ${PLATFORM}
.endif

.for dir in ${PLATFORM_DIRS}
CPPFLAGS+=-I${dir}
.endfor

# platform sources
PLATFORM_SRCS!=find ${PLATFORM_DIRS} -name '*.c' ! -name 'main.c' | \
	awk -F/ '!seen[$$NF]++'

# VM sources
VM_SRCS!=find allocator state vm -name '*.c'

SRCS=${VM_SRCS} ${PLATFORM_SRCS} ${MAIN}
OBJS=${SRCS:.c=.o}
DEPS=${SRCS:.c=.d}

# targets
all: ${PROGRAM}

${PROGRAM}: ${OBJS}
	@mkdir -p ${.TARGET:H}
	${ANALYZER} ${CC} ${CFLAGS} ${LDFLAGS} -o ${.TARGET} ${OBJS}

# compilation
.c.o:
	${ANALYZER} ${CC} ${CPPFLAGS} ${CFLAGS} -MMD -MP -c -o ${.TARGET} ${.IMPSRC}

# header dependencies
.for dep in ${DEPS}
.if exists(${dep})
.include "${dep}"
.endif
.endfor

# cleaning
clean:
	rm -rf bin
	rm -f ${OBJS} ${DEPS}

.PHONY: all clean
