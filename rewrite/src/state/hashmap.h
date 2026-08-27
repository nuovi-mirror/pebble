#pragma once

#include "../platform/use/lcalloc.h"
#include "../platform/use/lalloc.h"
#include "../platform/use/lfree.h"
#include "../platform/use/exitproc.h"
#include "../platform/use/print.h"
#include "../platform/use/cmpstr.h"

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

static const unsigned long HASHMAPBASE  = 0x811c9dc5;
static const unsigned long HASHMAPPRIME = 0x01000193;

static unsigned long mapHash(SHashMap *m, char *str) {
	unsigned long h = HASHMAPBASE;
	unsigned long inital = 0;

	while (*str) {
		inital ^= (unsigned char)*str++;
		inital *= HASHMAPPRIME;
	}

	return inital & (m->cap - 1); /* must be power of two */
}

static SHashMap initHashMap(unsigned long cap) {
	SHashMap m;
	m.size = 0;
	m.cap = cap;
	m.buckets = lcalloc(cap, sizeof(HashMapEntry *));

	if (m.buckets == NULL) {
		print("ERRORL HASHMAP: FATA: ALLOCATION FAILED!\n");
		exitproc(1);
	}

	return m;
}

static void hashMapPut(SHashMap *m, char *key, void *value) {
	unsigned long idx = mapHash(m, key);
	HashMapEntry *e = m->buckets[idx];
	while (e != NULL) {
		if (cmpstr(e->key, key) == 0) {
			e->value = value;
			return;
		}

		e = e->next;
	}

	/* not found - prep new entry */
	HashMapEntry *entry = lalloc(sizeof(HashMapEntry));

	if (entry == NULL) {
		print("ERROR: HASHMAP: FATAL: ALLOCATION FAILED!\n");
		exitproc(1);
	}

	entry->key = key;
	entry->value = value;
	entry->next = m->buckets[idx];
	m->buckets[idx] = entry;
	m->size++;
}

static void *hashMapGet(SHashMap *m, char *key) {
	HashMapEntry *e = m->buckets[mapHash(m, key)];

	while (e != NULL) {
		if (cmpstr(e->key, key) == 0)
			return e->value;
		
		e = e->next;
	}
	
	return NULL;
}

static int hashMapRemove(SHashMap *m, char *key) {
	unsigned long idx = mapHash(m, key);

	HashMapEntry *e = m->buckets[idx];
	HashMapEntry *prev = NULL;

	while (e != NULL) {
		if (cmpstr(e->key, key) == 0) {
			if (prev == NULL)
				m->buckets[idx] = e->next;
			else
				prev->next = e->next;

			lfree(e);
			m->size--;
			return 1;
		}

		prev = e;
		e = e->next;
	}

	return 0;
}

static void hashMapFreeMap(SHashMap *m) {
	for (unsigned long i = 0; i < m->cap; i++) {
		HashMapEntry *e = m->buckets[i];

		while (e != NULL) {
			HashMapEntry *next = e->next;
			lfree(e);
			e = next;
		}
	}

	lfree(m->buckets);
	m->buckets = NULL;
	m->size = 0;
	m->cap = 0;
}
