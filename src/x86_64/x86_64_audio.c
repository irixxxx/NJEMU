#include <stdio.h>
#include <stdlib.h>

#include "common/audio_driver.h"

typedef struct x86_64_audio {
	int32_t channel;
} x86_64_audio_t;

static void *x86_64_init(void) {
	x86_64_audio_t *x86_64 = (x86_64_audio_t*)calloc(1, sizeof(x86_64_audio_t));
	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_audio_t *x86_64 = (x86_64_audio_t*)data;

	free(x86_64);
}

static int32_t x86_64_volumeMax(void *data) {
	return 100;
}

static bool x86_64_chSRCReserve(void *data, int32_t samples, int32_t freqency, int32_t channels) {
	x86_64_audio_t *x86_64 = (x86_64_audio_t*)data;

    x86_64->channel = 1;
	return x86_64->channel >= 0;
}

static bool x86_64_chReserve(void *data, int32_t samplecount, int32_t channels) {
	x86_64_audio_t *x86_64 = (x86_64_audio_t*)data;
	// int32_t format = channels == 1 ? PS2_AUDIO_FORMAT_MONO : PS2_AUDIO_FORMAT_STEREO;
	// x86_64->channel = sceAudioChReserve(PS2_AUDIO_NEXT_CHANNEL, samplecount, format);
	// return x86_64->channel >= 0;
	return false;
}

static void x86_64_release(void *data) {
}

static void x86_64_srcOutputBlocking(void *data, int32_t volume, void *buffer) {
}

static void x86_64_outputPannedBlocking(void *data, int leftvol, int rightvol, void *buf) {
	x86_64_audio_t *x86_64 = (x86_64_audio_t*)data;
}

audio_driver_t audio_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_volumeMax,
	x86_64_chSRCReserve,
	x86_64_chReserve,
	x86_64_srcOutputBlocking,
	x86_64_outputPannedBlocking,
	x86_64_release,
};