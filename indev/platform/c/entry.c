/* portable entry point helper (C) */

#include <time.h> /* used for psedo-random helper */
#include <stdio.h> /* needed for debug */

#include "entry.h"
#include "print.h"
#include "exitproc.h"
#include "mem.h"

/* temporary NULL definition we can use */
#undef NULL
#define NULL ((void *)0)

/* used for psedo-random helper */
static volatile unsigned long sink;

/* stack helpers */
void pushframe
(Stack *stack, StackFrame *frame) 
{
	if (stack->count >= stack->capacity) 
	{
		print("ERROR: CALLSTACK: STACK OVERFLOW!\n");
		exitproc(1);
	}

	stack->items[stack->count] = *frame;
	stack->count++;
}

StackFrame popframe
(Stack *stack) 
{
	StackFrame frame;

	if (stack->count == 0) 
	{
		print("ERROR: CALLSTACK: STACK UNDERFLOW!\n");
		exitproc(1);
	}

	stack->count--;
	frame = stack->items[stack->count];
	
	return frame;
}

StackFrame readframe
(Stack *stack) 
{
	StackFrame frame;

	if (stack->count == 0) 
	{
		print("ERROR: CALLSTACK: STACK UNDERFLOW!\n");
		exitproc(1);
	}

	return frame;
}

Args initargs
(int argc, char **argv) 
{
	Args cliargs;

	cliargs.count = (unsigned long)argc;
	cliargs.values = argv;

	return cliargs;
}

Stack *initstack
(unsigned long capacity) 
{
	StackFrame *stack_items = (StackFrame *)mem_stack_callStack_frames;
	Stack *stack = (Stack *)mem_stack_callStack_stack;

	if (stack == NULL || stack_items == NULL || capacity == 0)
	{
		print("ERROR: INIT: CANNOT ALLOCATE A CALL STACK!\n");
		exitproc(1);
	}

	stack->items = stack_items;
	stack->count = 0;
	stack->capacity = capacity; /* max number of entries on the stack at a time */

	return stack;
}

void freestack
(Stack *stack)
{
	/* XXX this does nothing now */
}

/* random number generation */
static void
work(unsigned long long r, unsigned long long s, 
		unsigned long a, unsigned long b, unsigned long c)
{
    unsigned long long x = r;

    for (unsigned long long i = 0; i < s; ++i) {
        x ^= x << a;
        x ^= x >> b;
        x ^= x << c;
        x += i;
    }

     sink ^= x;
}

static unsigned long long
mix64(unsigned long long x)
{
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;

    return x;
}

unsigned long long ground
(unsigned long long w, unsigned long long s, unsigned long a,
 unsigned long b, unsigned long c)
{	
	clock_t t1, t2;
	clock_t d1, d2, d3, d4;

	t1 = clock();
	work(w, s, a, b, c);
	t2 = clock();
	d1 = t2 - t1;
	
	return d1;
}

static void
shuffle(unsigned int order[4], unsigned long long *state)
{
    unsigned int i;
    unsigned int j;
    unsigned int tmp;

    for (i = 0; i < 4; ++i)
        order[i] = i;

    for (i = 3; i > 0; --i) {
        *state = mix64(*state);

        j = (unsigned int)(*state % (i + 1));

        tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }
}

unsigned long long grun
(void)
{
	unsigned int wsize = 4;
	unsigned int order[wsize];

	unsigned long long d[wsize];
	unsigned long long r;
	unsigned long long seed;
	
	unsigned long long w1 = 0x9E3779B97F4A7C15ULL;
	unsigned long long w2 = 0xBF58476D1CE4E5B9ULL;
	unsigned long long w3 = 0x6D2B79F5A5A5A5A5ULL;
	unsigned long long w4 = 0xFF51AFD7ED558CCDULL;

	r = ground(w1, 10000000ULL, 13, 29, 17);
	r = mix64(r);
	shuffle(order, &r);

	for (unsigned int i = 0; i < wsize; ++i) {
		switch (order[i]) {
			case 0:
				d[i] = ground(w1, 10000000ULL, 13, 29, 17);
				break;
			case 1:
				d[i] = ground(w2, 10000000ULL, 21, 37, 11);
				break;
			case 2:
				d[i] = ground(w3, 10000000ULL, 31, 23, 41);
				break;
			case 3:
				d[i] = ground(w4, 10000000ULL, 43, 19, 27);
				break;
		}
	}

	/* XXX debug
	printf("order %u %u %u %u: ",
	    order[0], order[1], order[2], order[3]);

	printf("%llu %llu %llu %llu\n",
	    d[0], d[1], d[2], d[3]);

	printf(
		"%llu %llu %llu %llu\n",
		(unsigned long long)d[0],
	    	(unsigned long long)d[1],
	    	(unsigned long long)d[2],
	    	(unsigned long long)d[3]);
	*/

	seed  = mix64((unsigned long long)d[0] + w1);
	seed ^= mix64((unsigned long long)d[1] + w2);
	seed ^= mix64((unsigned long long)d[2] + w3);
	seed ^= mix64((unsigned long long)d[3] + w4);

	return seed;
}

unsigned long long grandom
(void)
{
	int probe;
	unsigned long long addr_entropy = (unsigned long long)(unsigned long)&probe;

	unsigned long long seed  = grun();
	int i;
	int c = 0; /* change this for more mixing */

	for (i = 0; i < c; i++)
	{ seed  = mix64(seed ^ grun()); }

	seed ^= mix64(addr_entropy);

	return seed;
}

#undef NULL
