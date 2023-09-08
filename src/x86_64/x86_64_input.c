#include <stdio.h>
#include <stdlib.h>
#include "common/input_driver.h"

typedef struct x86_64_input {
} x86_64_input_t;

static void *x86_64_init(void) {
	x86_64_input_t *x86_64 = (x86_64_input_t*)calloc(1, sizeof(x86_64_input_t));
	return x86_64;
}

static void x86_64_free(void *data) {
	free(data);
}

static uint32_t x86_64_poll(void *data) {
	uint32_t btnsData = 1;
	return btnsData;
}

#if (EMU_SYSTEM == MVS)
static uint32_t x86_64_pollFatfursp(void *data) {
	uint32_t btnsData = 0;
	return btnsData;
}

static uint32_t x86_64_pollAnalog(void *data) {
	uint32_t btnsData;
	return btnsData;
}
#endif


input_driver_t input_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_poll,
#if (EMU_SYSTEM == MVS)
	x86_64_pollFatfursp,
	x86_64_pollAnalog,
#endif
};