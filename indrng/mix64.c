#include "indrng.h"

unsigned long long mix64 
(unsigned long long x)
{
	x ^= x >> 30;
	x *= 0xBF58476D1CE4E5B9ULL;
	x ^= x >> 27;
	x *= 0x94D049BB133111EBULL;
	x ^= x >> 31;

	return x;
}
