#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <timer.h>
#include "common/ticker_driver.h"

typedef struct x86_64_ticker {
} x86_64_ticker_t;

static void *x86_64_init(void) {
	x86_64_ticker_t *x86_64 = (x86_64_ticker_t*)calloc(1, sizeof(x86_64_ticker_t));
	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_ticker_t *x86_64 = (x86_64_ticker_t*)data;
	free(x86_64);
}

static u_int64_t x86_64_currentMs(void *data) {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return (start.tv_sec * 1000) + (start.tv_nsec / 1000000);
}

ticker_driver_t ticker_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_currentMs,
};