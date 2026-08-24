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

