#ifndef STATE_HASHMAP_H_
#define STATE_HASHMAP_H_

#include "allocator.h"

typedef struct HashMapEntry {
	char *key;
	void *value;
	struct HashMapEntry *next;
} HashMapEntry;

typedef struct SHashMap {
	unsigned long size;					/* number of entries */
	unsigned long cap;					/* number of buckets */
	void **buckets;						/* array of chain heads */
} SHashMap;

#define HASHMAPBASE 0x811c9dc5;
#define HASHMAPPRIME 0x01000193;

unsigned long mapHash
(SHashMap *m, const char *str);

SHashMap initHashMap
(unsigned long cap, Arena *persistAlloc);

void hashMapPut
(SHashMap *m, const char *key, const void *value, Arena *persistAlloc);

void *hashMapGet
(SHashMap *m, const char *key);

int hashMapRemove
(SHashMap *m, const char *key);

void hashMapFreeMap
(SHashMap *m);

#endif
