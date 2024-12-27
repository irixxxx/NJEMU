#include <stdio.h>
#include <stdlib.h>
#include "common/power_driver.h"

typedef struct x86_64_power {
} x86_64_power_t;

static void *x86_64_init(void) {
	x86_64_power_t *x86_64 = (x86_64_power_t*)calloc(1, sizeof(x86_64_power_t));
	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_power_t *x86_64 = (x86_64_power_t*)data;
	free(x86_64);
}

static int32_t x86_64_batteryLifePercent(void *data) {
	return 100;
}

static bool x86_64_IsBatteryCharging(void *data) {
	return true;
}

static void x86_64_setCpuClock(void *data, int32_t clock) {
}

static void x86_64_setLowestCpuClock(void *data) {
}

static int32_t x86_64_getHighestCpuClock(void *data) {
	return 1;
}

power_driver_t power_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_batteryLifePercent,
	x86_64_IsBatteryCharging,
	x86_64_setCpuClock,
	x86_64_setLowestCpuClock,
	x86_64_getHighestCpuClock,
};