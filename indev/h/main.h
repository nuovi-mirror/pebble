#ifndef h_main_
#define h_main_

#undef NULL
#define NULL ((void *)0)

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

#endif
