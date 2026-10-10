#define wsize (8) /* machine-size word */
#define scount (16) /* number of seeds in a pool */
#define mcount (2) /* times to mix each seed */
#define ecount (4) /* number of 64bit seeds in the seed struct */

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
{
	unsigned long long value[ecount];
};

extern volatile unsigned long long sink;

unsigned long long mix64 (unsigned long long x);
void shuffle (unsigned int order[wsize], unsigned long long *state);
struct seed grandom (void);
