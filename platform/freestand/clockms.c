#ifdef noclock
#define platform_clockms_

unsigned long clockms
(void)
{
	static unsigned long pebble_clockms_ticks_;
	return pebble_clockms_ticks_++;
}

#endif
