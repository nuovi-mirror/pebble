#ifndef VM_H_
#define VM_H_

#include "entry.h"

/* call this to enter the VM code */
int vmmain(Args cliargs, Stack* stack, unsigned long long seed);

#endif
