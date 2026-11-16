#ifndef h_lalloc_
#define h_lalloc_
#if defined(debug) || defined(memlog)
extern unsigned long lallocTracker;
#endif
void *lalloc (unsigned long size, unsigned int max);
#endif
