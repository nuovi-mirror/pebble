#include "indrng.h"
#include <time.h>

volatile unsigned long long sink;

static unsigned long long winit[] = {
	0x243F6A8885A308D3ULL,
	0x13198A2E03707344ULL,
	0xA4093822299F31D0ULL,
	0x082EFA98EC4E6C89ULL,
	0x452821E638D01377ULL,
	0xBE5466CF34E90C6CULL,
	0xC0AC29B7C97C50DDULL,
	0x3F84D5B5B5470917ULL
};

static unsigned long long wlinit[][3] = {
	{13, 7, 17},
	{17,31,  8},
	{23,17, 26},
	{26,21,  9},
	{29,17, 13},
	{37,11, 43},
	{41,27, 17},
	{47,19, 23}
};

#define wicount (sizeof(winit) / sizeof(winit[0]))
#define wlicount (sizeof(wlinit) / sizeof(wlinit[0]))

static void work 
(unsigned long long r, unsigned long long s, unsigned long a, unsigned long b, unsigned long c)
{
	unsigned long long x = r;

	for (unsigned long long i = 0; i < s; ++i) {
		x ^= x << a;
		x ^= x >> b;
		x ^= x << c;
		x += i;
	}

	sink ^= x;
}

static unsigned long long ground 
(unsigned long long w, unsigned long long s, unsigned long a, unsigned long b, unsigned long c)
{
	clock_t t1, t2, d1;

	t1 = clock();
	work(w, s, a, b, c);
	t2 = clock();
	d1 = t2 - t1;

	return (unsigned long long)d1; /* XXX please be safe */
}

static unsigned long long grun 
(void)
{
	unsigned int order[wsize];
	unsigned long long work[wsize];
	unsigned long long times[wsize];
	unsigned long long workloads[wsize][3];

	unsigned long long d[wsize];
	unsigned long long r;
	unsigned long long seed;

	for (unsigned long i = 0; i < wsize; i++)
		work[i] = winit[i % wicount];

	for (unsigned long i = 0; i < wsize; i++) {
		workloads[i][0] = wlinit[i % wlicount][0];
		workloads[i][1] = wlinit[i % wlicount][1];
		workloads[i][2] = wlinit[i % wlicount][2];
	}

	seed = ground(work[0], 10000001ULL, workloads[0][0], workloads[1][1],
		workloads[2][2]);

	for (unsigned int i = 0; i < wsize; i++)
		times[i] = 1000000ULL + (mix64(seed+i) % 20000000ULL);

	r = ground(work[0], times[0], workloads[0][0], workloads[0][1], workloads[0][2]);
	r = mix64(r);
	shuffle(order, &r);

	for (unsigned int i = 0; i < wsize; ++i)
		d[i] = ground(work[order[i]], times[i], workloads[order[i]][0],
			workloads[order[i]][1], workloads[order[i]][2]);

	for (int i = 0; i < wsize; i++)
		seed = mix64((seed ^ d[i]) + work[i]);

	return seed;
}

struct seed grandom 
(void)
{
	unsigned long long seeds[scount];
	struct seed seed = { 0 };

	/* generate seeds */
	for (unsigned long i = 0; i < scount; i++)
		seeds[i] = grun();

	/* mix seeds */
	for (unsigned long i = 0; i < mcount; i++)
		seeds[i % scount] = mix64(seeds[i % scount] ^ grun());

	for (unsigned long i = 0; i < ecount; i++)
		seed.value[i] = 0;

	for (unsigned long i = 0; i < scount; i++)
		seed.value[i % ecount] = mix64(seed.value[i % ecount] ^ seeds[i]);

	return seed;
}
