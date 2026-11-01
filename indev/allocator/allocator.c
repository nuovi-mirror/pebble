#include "allocator.h"
#include "ffillmem.h"
#include "main.h"

static unsigned long align_up
(unsigned long n)
{
	unsigned long misalign = n % ARENA_ALIGNMENT;
	return misalign ? (n + (ARENA_ALIGNMENT - misalign)) : n;
}

unsigned long arenaBackingSize
(unsigned long usable)
{
	return align_up(sizeof(struct Arena)) + (ARENA_ALIGNMENT - 1) + usable;
}

struct Arena *initAlloc
(void *backing, unsigned long backing_size)
{
	/* platform layer gives us backing memory */
	if (backing == NULL) error(errfatal, errcodeallocatorerror, "Given a null backing pointer!");

	unsigned long addr = (unsigned long)backing;
	unsigned long overhead = align_up(addr + sizeof(struct Arena)) - addr;

	if (backing_size < overhead) error(errfatal, errcodeallocatorerror, "Backing allocation is too small!");

	struct Arena *arena = (struct Arena *)backing;
	char *srt_ptr = (char *)backing + overhead;

	arena->size = backing_size - overhead;
	arena->curr_ptr = srt_ptr;
	arena->arena_ptr = srt_ptr;

#ifndef nozeromem
	/* initalize the memory to 0 */
	ffillmem(0, (volatile void *)arena->arena_ptr, arena->size);
#endif

	return arena; /* return a pointer to the new allocated arena */
}

void *alloc
(struct Arena *arena, unsigned long size)
{
	unsigned long used = getAllocatorSizeUsed(arena);
	unsigned long left = arena->size - used; /* XXX cannot underflow; used <= size */
	unsigned long misalign = used % ARENA_ALIGNMENT;
	unsigned long padding = misalign ? (ARENA_ALIGNMENT - misalign) : 0;

	if (padding > left || size > left - padding) {
		error(errfatal, errcodeallocationerror, "Out of memory!");
		return NULL; /* XXX never hit */
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
	return;
}

void resetAllocator 
(struct Arena *arena) 
{
	if (arena == NULL) return;

	/* XXX dont zero it - returned memory may contain old data */
	arena->curr_ptr = arena->arena_ptr;
	return;
}

unsigned long getAllocatorSizeUsed 
(struct Arena *arena) 
{ return (unsigned long)(arena->curr_ptr - arena->arena_ptr); }

unsigned long getAllocatorSizeRemaining 
(struct Arena *arena) 
{ return arena->size - getAllocatorSizeUsed(arena); }
