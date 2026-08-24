#pragma once

#include <unistd.h>
#include <stdlib.h>

#include "../platform/use/print.h"

typedef struct Arena {				/* template for an arena */
	char *curr_ptr;				/* pointer to the next free block */
	char *arena_ptr;			/* pointer to the start of the arena */
	unsigned long size;			/* size of the arena in bytes */
} Arena;

struct Arena *initAlloc(size_t size) {		/* function to init an arena 
						   takes the size of the arena to init
						   returns a pointer to the Arena struct */
	char *srt_ptr = malloc(size);		/* allocate the arena */
	
	if (srt_ptr == NULL) {
		print("ERROR: ALLOCATOR: NONFATAL: GIVEN NULL ALLOCATOR POINTER\n");
		return NULL;
	}

	struct Arena *arena = malloc(sizeof(struct Arena));
	
	if (arena == NULL) {			/* if we cannot allocate the area */
		free(srt_ptr);			/* free the are we already allocated */
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

char *alloc(struct Arena *arena, unsigned long size) {	
						/* function to allocate data on an arena
						   takes the arena to allocate against and
						   the size of the data to allocate
						   returns a pointer to the area usable */
	unsigned long used = (unsigned long)(arena->curr_ptr - arena->arena_ptr);
						/* get the size used */
	if (used + size > arena->size) {
		print("ERROR: ALLOCATOR: FATAL: OUT OF MEMORY!");
		exit(1);			/* exit with OOM error */
	}

	char *result = arena->curr_ptr;
	arena->curr_ptr = arena->curr_ptr + size;
						/* set the new pointer */
	return result;
}

void freeAllocator(struct Arena *arena) {
	freezero(arena->arena_ptr, arena->size);
}

/* example

int main() {
	struct Arena *arena = initArena(32);	init an arena of 32 bytes
	
	if (arena == NULL)
		exit(1);

	char *data = "stuff\n";
	char *ptr = alloc(arena, 12); 		grab a slice of 12 bytes
						allocate a spot for the data

	for (int i = 0; i < getstrlen(data) + 1; i++)
		ptr[i] = data[i];

	print(ptr);
	free(arena);
	return 0;
}

*/
