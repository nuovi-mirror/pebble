#include "allocator.h"
#include "exitproc.h"
#include "main.h"
#include "print.h"

struct Arena *initAlloc (void *backing, unsigned long size) {
	/* char *srt_ptr = lalloc(size); XXX this */
	char *srt_ptr; /* XXX hack */

	if (backing == NULL) {
		print("ERROR: ALLOCATOR: NONFATAL: GIVEN NULL BACKING POINTER\n");
		return NULL;
	}

	/* struct Arena *arena = lalloc(sizeof(struct Arena)); XXX this */
	struct Arena *arena = backing;
	srt_ptr = (char *)backing + sizeof(struct Arena);

	if (arena == NULL) { /* if we cannot allocate the area */
		/* lfree(srt_ptr); XXX not needed now */
		print("ERROR: ALLOCATOR: ALLOCATOR INIT FAILED\n");
		return NULL; /* return null */
	}
	/* create the arena */
	arena->size = size;	    /* set the size */
	arena->curr_ptr = srt_ptr;  /* set the current pointer */
	arena->arena_ptr = srt_ptr; /* set the start of the arena */

	return arena; /* return a pointer to the new
			 allocated arena */
}

void *alloc (struct Arena *arena, unsigned long size) {
	unsigned long used = (unsigned long)(arena->curr_ptr - arena->arena_ptr);
	unsigned long misalign = used % ARENA_ALIGNMENT;
	unsigned long padding = misalign ? (ARENA_ALIGNMENT - misalign) : 0;
	
	if (used + padding + size > arena->size) {
		print("ERROR: ALLOCATOR: FATAL: OUT OF MEMORY!\n");
		exitproc(1); /* exit with OOM error */
	}

	arena->curr_ptr += padding;
	void *result = arena->curr_ptr;
	arena->curr_ptr += size;
	return result;
}

void freeAllocator (struct Arena *arena) {
	/* XXX does nothing now since these are allocated on BSS */
}

void resetAllocator (struct Arena *arena) {
	if (arena == NULL)
		return;

	arena->curr_ptr = arena->arena_ptr;
	return;
}

unsigned long getAllocatorSizeUsed (struct Arena *arena) 
{ return (unsigned long)(arena->curr_ptr - arena->arena_ptr); }

unsigned long getAllocatorSizeRemaining (struct Arena *arena) 
{ return arena->size - getAllocatorSizeUsed(arena); }
