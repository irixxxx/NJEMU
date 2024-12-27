#include <stdio.h>
#include <stdlib.h>
#include "common/thread_driver.h"

#include <SDL.h>

typedef struct x86_64_thread {
    SDL_Thread *thread;
    SDL_sem *start;
    SDL_sem *end;
    int32_t (*threadFunc)(uint32_t, void *);
} x86_64_thread_t;


static int childThread(void *arg)
{
    int32_t res;
    x86_64_thread_t *x86_64 = (x86_64_thread_t *)arg;
    SDL_SemWait(x86_64->start);
    res = x86_64->threadFunc(0, NULL);
    SDL_SemPost(x86_64->end);
    return res;
}

static void *x86_64_init(void) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)calloc(1, sizeof(x86_64_thread_t));
	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
	free(x86_64);
}

static bool x86_64_createThread(void *data, const char *name, int32_t (*threadFunc)(u_int32_t, void *), uint32_t priority, uint32_t stackSize) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
    x86_64->threadFunc = threadFunc;
    x86_64->start = SDL_CreateSemaphore(0);
    x86_64->end = SDL_CreateSemaphore(0);
    x86_64->thread = SDL_CreateThreadWithStackSize(childThread, name, stackSize, x86_64);

	return x86_64->thread != NULL;
}

static void x86_64_startThread(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
    SDL_SemPost(x86_64->start);
}

static void x86_64_waitThreadEnd(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
    SDL_SemPost(x86_64->end);
}

static void x86_64_wakeupThread(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
}

static void x86_64_deleteThread(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
}

static void x86_64_resumeThread(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
}

static void x86_64_suspendThread(void *data) {
	x86_64_thread_t *x86_64 = (x86_64_thread_t*)data;
}

static void x86_64_sleepThread(void *data) {
}

static void x86_64_exitThread(void *data, int32_t exitCode) {
}

thread_driver_t thread_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_createThread,
	x86_64_startThread,
	x86_64_waitThreadEnd,
	x86_64_wakeupThread,
	x86_64_deleteThread,
	x86_64_resumeThread,
	x86_64_suspendThread,
	x86_64_sleepThread,
	x86_64_exitThread
};
