#include "allocator.h"
#include "ffillmem.h"
#include "exitproc.h"
#include "main.h"
#include "print.h"

struct Arena *initAlloc 
(void *backing, unsigned long size) 
{
	/* platform layer gives us backing memory */
	if (backing == NULL) {
		print("ERROR: ALLOCATOR: GIVEN NULL BACKING POINTER\n");
		exitproc(1);
	}

	struct Arena *arena = (struct Arena *)backing;
	char *srt_ptr = (char *)backing + sizeof(struct Arena);

	if (size < sizeof(struct Arena)) 
	{
		print("ERROR: ALLOCATOR: BACKING IS TOO SMALL!\n");
		exitproc(1);
	}

	/* create the arena */
	arena->size = size; 	/* set the size */
	arena->curr_ptr = srt_ptr;  /* set the current pointer */
	arena->arena_ptr = srt_ptr; /* set the start of the arena */

#ifdef nozeromem
	/* initalize the memory to 0 */
	ffillmem(0, (volatile void *)arena->arena_ptr, arena->size);
#endif

	return arena; /* return a pointer to the new allocated arena */
}

void *alloc 
(struct Arena *arena, unsigned long size) 
{
	unsigned long used = (unsigned long)(arena->curr_ptr - arena->arena_ptr);
	unsigned long misalign = used % ARENA_ALIGNMENT;
	unsigned long padding = misalign ? (ARENA_ALIGNMENT - misalign) : 0;

	/* OOM error */
	if (used + padding + size > arena->size) {
		print("ERROR: ALLOCATOR: FATAL: OUT OF MEMORY!\n");
		exitproc(1); 
	}

	arena->curr_ptr += padding;
	void *result = arena->curr_ptr;
	arena->curr_ptr += size;
	return result;
}

void freeAllocator 
(struct Arena *arena) /* yes, i know this is unused */
{
	/* XXX does nothing now since the backings are now managed by the platform
	 * abstraction layer - it is their job to free it */
}

void resetAllocator 
(struct Arena *arena) 
{
	if (arena == NULL)
		return;

	/* XXX dont zero it - returned memory may contain old data */
	arena->curr_ptr = arena->arena_ptr;
	return;
}

unsigned long getAllocatorSizeUsed 
(struct Arena *arena) 
{ 
	return (unsigned long)(arena->curr_ptr - arena->arena_ptr); 
}

unsigned long getAllocatorSizeRemaining 
(struct Arena *arena) 
{ 
	return arena->size - getAllocatorSizeUsed(arena); 
}
