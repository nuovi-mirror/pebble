#include "indrng.h"

void shuffle 
(unsigned int order[wsize], unsigned long long *state)
{
	unsigned int i;
	unsigned int j;
	unsigned int tmp;

	for (i = 0; i < wsize; ++i)
		order[i] = i;

	for (i = wsize - 1; i > 0; --i) {
		*state = mix64(*state);

		j = (unsigned int)(*state % (i + 1));

		tmp = order[i];
		order[i] = order[j];
		order[j] = tmp;
	}
}
