#include <stdlib.h>
#include <string.h>

#include "../platform/use/print.h"

typedef struct VarEntry {
	char *key;
	void *value;
	struct VarEntry *next;
} Entry;

typedef struct VarMap {
	size_t size;					/* number of entries */
	size_t cap;					/* number of buckets */
	void **buckets;					/* array of chain heads */
} Map;

const size_t BASE = 0x811c9dc5;
const size_t PRIME = 0x01000193;

size_t hashVar(struct VarMap *m, char *str) {
	size_t inital = BASE;
	while(*str) {
		inital ^= (unsigned char)*str++;
		inital *= PRIME;
	}

	return inital & (m->cap - 1);
}

struct VarMap initVars(size_t cap) {
	struct VarMap m;
	m.size = 0;
	m.cap = cap;
	m.buckets = calloc(cap, sizeof(Entry *));
	
	if (m.buckets == NULL) {
		print("ERROR: VARIABLES: FATAL: ALLOCATION FAILED!\n");
		exit(1);
	}

	return m;
}

void putVar(struct VarMap *m, char *str, void *value) {
	size_t idx = hashVar(m, str);
	Entry *e = m->buckets[idx];

	while (e != NULL) {
		if (strcmp(e->key, str) == 0) {
			e-> value = value; /* override existing */
			return;
		}

		e = e->next;
	}

	/* not found - prepend new entry */
	Entry *entry = malloc(sizeof(Entry));

	if (entry == NULL) {
		print("ERROR: VARIABLES: FATAL: ALLOCATION FAILED!\n");
		exit(1);
	}

	entry->key = str;
	entry->value = value;
	entry->next = m->buckets[idx];
	m->buckets[idx] = entry;
	m->size++;
}

void *getVar(struct VarMap *m, char *str) {
	Entry *e = m->buckets[hashVar(m, str)];

	while (e != NULL) {
		if (strcmp(e->key, str) == 0)
			return e->value;
		e = e->next;
	}

	return NULL;

}

void freeVars(struct VarMap *m) {
	for (size_t i = 0; i < m->cap; i++) {
		Entry *e = m->buckets[i];
		while (e != NULL) {
			Entry *next = e->next;
			free(e);
			e = next;
		}
	}

	free(m->buckets);
	m->buckets = NULL;
	m->size = 0;
	m->cap = 0;
}
