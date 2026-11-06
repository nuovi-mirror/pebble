#include "variables.h"
#include "allocator.h"
#include "hashmap.h"
#include "main.h"

unsigned long hashVar (VarMap *m, const char *str)
{ return mapHash(m->map, str); }

VarMap initVars 
(unsigned long cap, Arena *persistAlloc)
{ 
	VarMap vmap;
	SHashMap *map = alloc(persistAlloc, sizeof(SHashMap));
	*map = initHashMap(cap, persistAlloc); 
	void **arr = alloc(persistAlloc, cap * sizeof(void *));
	for (unsigned long i = 0; i < cap; i++) arr[i - 1] = NULL;

	vmap.map = map;
	vmap.arr = arr;
	vmap.max = cap;
	vmap.arrcount = 0;
	vmap.mapcount = 0;	

	return vmap;
}

void putVar 
(VarMap *m, const char *key, const void *value, Arena *persistAlloc) 
{
	if (m->arrcount + m->mapcount >= m->max) error(errfatal, errcodelimitreached, "Too many variables created!");
	hashMapPut(m->map, key, value, persistAlloc);
	m->mapcount++;
}

void *getVar (VarMap *m, const char *key)
{ return hashMapGet(m->map, key); }

void freeVars (VarMap *m)
{
	hashMapFreeMap(m->map); 
	for (unsigned long i = 0; i < m->max; i++) m->arr[i] = NULL;
}
