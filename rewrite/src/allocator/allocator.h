#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include "../platform/use/print.h"
#include "../platform/use/exitproc.h"
#include "../platform/use/lalloc.h"
#include "../platform/use/lfree.h"

typedef struct Arena {				/* template for an arena */
	char *curr_ptr;				/* pointer to the next free block */
	char *arena_ptr;			/* pointer to the start of the arena */
	unsigned long size;			/* size of the arena in bytes */
} Arena;

struct Arena *initAlloc(unsigned long size) {	/* function to init an arena 
						   takes the size of the arena to init
						   returns a pointer to the Arena struct */
	char *srt_ptr = lalloc(size);		/* allocate the arena */
	
	if (srt_ptr == NULL) {
		print("ERROR: ALLOCATOR: NONFATAL: GIVEN NULL ALLOCATOR POINTER\n");
		return NULL;
	}

	struct Arena *arena = lalloc(sizeof(struct Arena));
	
	if (arena == NULL) {			/* if we cannot allocate the area */
		lfree(srt_ptr);			/* free the are we already allocated */
		print("ERROR: ALLOCATOR: ALLOCATOR INIT FAILED\n");
		return NULL;			/* return null */
	}
						/* create the arena */
	arena->size = size;			/* set the size */
	arena->curr_ptr = srt_ptr;		/* set the current pointer */
	arena->arena_ptr = srt_ptr;		/* set the start of the arena */

	return arena;				/* return a pointer to the new allocated 
						   arena */
}

void *alloc(struct Arena *arena, unsigned long size) {	
						/* function to allocate data on an arena
						   takes the arena to allocate against and
						   the size of the data to allocate
						   returns a pointer to the area usable */
	unsigned long used = (unsigned long)(arena->curr_ptr - arena->arena_ptr);
						/* get the size used */
	if (used + size > arena->size) {
		print("ERROR: ALLOCATOR: FATAL: OUT OF MEMORY!\n");
		exitproc(1);			/* exit with OOM error */
	}

	char *result = arena->curr_ptr;
	arena->curr_ptr = arena->curr_ptr + size;
						/* set the new pointer */
	return result;
}

void freeAllocator(struct Arena *arena) {
	if (arena == NULL)
		return;

	lfree(arena->arena_ptr);
	lfree(arena);
}

void resetAllocator
(struct Arena *arena)
{
	if (arena == NULL)
		return;

	arena->curr_ptr = arena->arena_ptr;
	return;
}
	

unsigned long getAllocatorSizeUsed
(struct Arena *arena)
{ return (unsigned long)(arena->curr_ptr - arena->arena_ptr); }

unsigned long getAllocatorSizeRemaining
(struct Arena *arena)
{ return arena->size - getAllocatorSizeUsed(arena); }

#endif
