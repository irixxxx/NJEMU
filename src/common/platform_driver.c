/******************************************************************************

	platform_driver.c

******************************************************************************/

#include <stddef.h>
#include "platform_driver.h"

platform_driver_t platform_null = {
	"null",
	NULL,
	NULL,
	NULL,
	NULL,
};

platform_driver_t *platform_drivers[] = {
#ifdef PSP
	&platform_psp,
#endif
	&platform_null,
	NULL,
};
