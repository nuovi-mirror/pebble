_Noreturn void exitproc
(int status)
{
	/* this bit is not portable, so just
	 * hlt for now, assume htl exists on
	 * your ISA, as it does on most sane 
	 * ones. */

	__asm__ volatile ("cli");

	for (;;)
	{
		__asm__ volatile ("hlt");
	}
}

