#ifndef STATE_HASHMAP_H_
#define STATE_HASHMAP_H_

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
(SHashMap *m, char *str);

SHashMap initHashMap
(unsigned long cap);

void hashMapPut
(SHashMap *m, char *key, void *value);

void *hashMapGet
(SHashMap *m, char *key);

int hashMapRemove
(SHashMap *m, char *key);

void hashMapFreeMap
(SHashMap *m);

#endif
