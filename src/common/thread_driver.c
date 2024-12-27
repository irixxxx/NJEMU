/******************************************************************************

	thread_driver.c

******************************************************************************/

#include <stddef.h>
#include "thread_driver.h"

thread_driver_t thread_null = {
	"null",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

thread_driver_t *thread_drivers[] = {
#ifdef PSP
	&thread_psp,
#endif
#ifdef PS2
	&thread_ps2,
#endif
#ifdef X86_64
	&thread_x86_64,
#endif
	&thread_null,
	NULL,
};
