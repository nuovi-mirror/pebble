#ifndef h_allocator_
#define h_allocator_

typedef struct Arena {	    /* template for an arena */
	char *curr_ptr;	    /* pointer to the next free block */
	char *arena_ptr;    /* pointer to the start of the arena */
	unsigned long size; /* usable bytes, i.e. what alloc() may hand out */
} Arena;

/* widest commonly-aligned types; used to compute safe alignment boundaries */
typedef union {
	long long ll;
	long double ld;
	void *p;
} MaxAlign;

#define ARENA_ALIGNMENT (sizeof(MaxAlign))

unsigned long arenaBackingSize (unsigned long usable);

struct Arena *initAlloc (void *backing, unsigned long backing_size);
void *alloc (struct Arena *arena, unsigned long size);
void freeAllocator (struct Arena *arena);
void resetAllocator (struct Arena *arena);
unsigned long getAllocatorSizeUsed (struct Arena *arena);
unsigned long getAllocatorSizeRemaining (struct Arena *arena);

#endif /* #ifndef h_allocator_ */
