#define wsize (8) /* machine-size word */
#define scount (8) /* number of seeds in a pool */
#define mcount (1) /* times to mix each seed */
#define ecount (2) /* number of 64bit seeds in the seed struct */

#if wsize < 2
#error "wsize must be at least 2"
#endif

#if scount < 1
#error "scount must be at least 1"
#endif

#if ecount < 1
#error "ecount must be at least 1"
#endif

#if mcount >= ecount
#error "mcount must be < ecount"
#endif

struct seed
{ unsigned long long value[ecount]; };

unsigned long long mix64 (unsigned long long x);
void shuffle (unsigned int order[wsize], unsigned long long *state);
void grandom (struct seed *seed, volatile unsigned long long *sink); /* this is thread-safe */
