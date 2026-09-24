#ifndef h_vm_
#define h_vm_
#include "entry.h"

/* call this to enter the VM code */
int vmmain (Args cliargs, Stack *stack, unsigned long long seed);
#endif
