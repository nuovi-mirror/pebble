#ifndef h_entry_
#define h_entry_

#include "limits.h"
#include "getprogram.h"
#include "mem.h"

struct PackedLimits {
	struct Limits limits;
	struct Memory memory;
};

unsigned long long getrandom (void);
struct PackedLimits setlimits (Program *program);

#endif
