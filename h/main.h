#ifndef h_main_
#define h_main_

#include <limits.h>
#include <float.h>

#undef NULL
#undef TRUE
#undef FALSE

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

#define errfatal 0
#define errsevere 1
#define errmajor 2
#define errnonfatal 3
#define errwarning 4

#define errcodegeneric 0
#define errcodelimitreached 1
#define errcodemalformedinput 2
#define errcodeplatformspecific 3
#define errcodeallocatorerror 4
#define errcodeallocationerror 5
#define errcodenumericoverflow 6
#define errcodenumericunderflow 7
#define errcodestackerror 8
#define errcodeundefined 9
#define errcodeescapesequencenotfound 10

#if defined(build_default)
#define buildstr "default"
#elif defined(build_debug)
#define buildstr "debug"
#elif defined(build_small)
#define buildstr "small"
#elif defined(build_fast)
#define buildstr "fast"
#else 
#define buildstr "UNKNOWN"
#endif

#if defined(platform_c)
#define platformstr "c"
#elif defined(platform_posix)
#define platformstr "posix"
#elif defined(platform_openbsd)
#define platformstr "openbsd"
#elif defined(platform_puredarwin)
#define platformstr "puredarwin"
#else
#define platformstr "UNKNOWN"
#endif

#if defined(__x86_64__) || defined(__amd64__) || defined(_X_X64) || defined(_M_AMD64)
#define isastr "amd64"
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define isastr "i386"
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64__)
#define isastr "arm64"
#else
#define isastr "UNKNOWN"
#endif

struct Memory;

typedef struct Args {
        unsigned long count;
        char **values;
} Args;

typedef struct StackFrame {
        unsigned long return_pc;
        const char *funcname; /* function name this frame belongs to */
} StackFrame;

typedef struct Stack {
        StackFrame *items;
        unsigned long count;
        unsigned long capacity;
} Stack;

Stack *initstack (unsigned long capacity, struct Memory *mem);
void freestack (Stack *stack);
void pushframe (Stack *stack, StackFrame *frame);
StackFrame popframe (Stack *stack);
Args initargs (int argc, char **argv);

int main (int argc, char **argv);
void error(unsigned int severity, unsigned int code, const char *msg);

#endif
