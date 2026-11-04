#ifndef h_main_
#define h_main_

#undef NULL
#define NULL ((void *)0)

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
