#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

typedef struct Arena 
{						/* template for an arena */
	char *curr_ptr;				/* pointer to the next free block */
	char *arena_ptr;			/* pointer to the start of the arena */
	unsigned long size;			/* size of the arena in bytes */
} Arena;

/* widest commonly-aligned types */
/* used to compute safe alignment boundaries */
typedef union 
{
	long long ll;
	long double ld;
	void *p;
} MaxAlign;

#define ARENA_ALIGNMENT (sizeof(MaxAlign))

struct Arena *initAlloc
(unsigned long size);

void *alloc
(struct Arena *arena, unsigned long size); 

void freeAllocator
(struct Arena *arena); 

void resetAllocator
(struct Arena *arena);

unsigned long getAllocatorSizeUsed
(struct Arena *arena);

unsigned long getAllocatorSizeRemaining
(struct Arena *arena);

#endif
