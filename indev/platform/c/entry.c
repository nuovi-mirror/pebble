/* portable entry point helper (C) */

#include <time.h> /* used for psedo-random helper */

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

/* random number generation
 * low entropy, but a lot better than the
 * alternative, which is nothing. Uses a mix
 * of ALSR + timing jitter to generate
 * random 64bit seeds */
#define wsize 8

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
	clock_t t1, t2, d1;

	t1 = clock();
	work(w, s, a, b, c);
	t2 = clock();
	d1 = t2 - t1;
	
	return d1;
}

static void shuffle
(unsigned int order[wsize], unsigned long long *state)
{
    unsigned int i;
    unsigned int j;
    unsigned int tmp;

    for (i = 0; i < wsize; ++i)
        order[i] = i;

    for (i = wsize - 1; i > 0; --i) {
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
	unsigned int order[wsize];
	unsigned long long work[wsize];
	unsigned long long times[wsize];

	unsigned long long d[wsize];
	unsigned long long r;
	unsigned long long seed;
	
	work[0] = 0x9E3779B97F4A7C15ULL;
	work[1] = 0xBF58476D1CE4E5B9ULL;
	work[2] = 0x6D2B79F5A5A5A5A5ULL;
	work[3] = 0xFF51AFD7ED558CCDULL;
	work[4] = 0x9E3779B97F4A7C15ULL;
	work[5] = 0xBF58476D1CE4E5B9ULL;
	work[6] = 0x6D2B79F5A5A5A5A5ULL;
	work[7] = 0xFF51AFD7ED558CCDULL;

	unsigned long long workloads[wsize][3] = {
		{13, 29, 17},
		{21, 37, 11},
		{31, 23, 41},
		{43, 19, 47},
		{13, 29, 17},
		{21, 37, 11},
		{31, 23, 41},
		{43, 19, 47},
	};

	seed = ground(work[0], 10000001ULL, 
			workloads[0][0], workloads[1][1], workloads[2][2]);


	for (unsigned int i = 0; i < wsize; i++)
		times[i] = 5000000ULL + (mix64(seed + i) % 10000001ULL);

	r = ground(work[0], times[0], 
			workloads[0][0], workloads[0][1], workloads[0][2]);
	r = mix64(r);
	shuffle(order, &r);

	for (unsigned int i = 0; i < wsize; ++i)
		d[i] = ground(work[order[i]], times[i], 
				workloads[order[i]][0], 
				workloads[order[i]][1], 
				workloads[order[i]][2]);

	for (int i = 0; i < wsize; i++)
		seed  = mix64((seed ^ d[i]) + work[i]);

	return seed;
}

unsigned long long grandom
(void)
{
	unsigned long long seed  = grun();
	int i;
	int c = 0; /* change this for more mixing */

	for (i = 0; i < c; i++)
		seed  = mix64(seed ^ grun());

	/* mix with the randomization provided by ALSR */
	/* XXX add this in maybe; more portable without it and
	 * it seems to be fine just with timing jitter

	int probe;
	unsigned long long alsr_entropy = (unsigned long long)&probe;
	seed ^= mix64(seed ^ alsr_entropy);

	*/

	return seed;
}

#undef NULL
