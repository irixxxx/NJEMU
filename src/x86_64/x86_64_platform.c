#include "emumain.h"
#include <SDL.h>

typedef struct x86_64_platform {
	SDL_Window* window;
} x86_64_platform_t;

static void *x86_64_init(void) {
	x86_64_platform_t *x86_64 = (x86_64_platform_t*)calloc(1, sizeof(x86_64_platform_t));

	// Initialize SDL for video, audio, and controller subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return NULL;
    }

	// Create a window (width, height, window title)
    SDL_Window* window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return NULL;
	}

	x86_64->window = window;

	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_platform_t *x86_64 = (x86_64_platform_t*)data;

	// Clean up and quit
    SDL_DestroyWindow(x86_64->window);
    SDL_Quit();

	free(x86_64);
}

static void x86_64_main(void *data, int argc, char *argv[]) {
	x86_64_platform_t *x86_64 = (x86_64_platform_t*)data;
    
	getcwd(screenshotDir, sizeof(screenshotDir));
    strcat(screenshotDir, "/PICTURE");
    mkdir(screenshotDir, 0777);
#if	(EMU_SYSTEM == CPS1)
	strcat(screenshotDir, "/CPS1");
#endif
#if	(EMU_SYSTEM == CX86_64)
	strcat(screenshotDir, "/CX86_64");
#endif
#if	(EMU_SYSTEM == MVS)
	strcat(screenshotDir, "/MVS");
#endif
#if	(EMU_SYSTEM == NCDZ)
	strcat(screenshotDir, "/NCDZ");
#endif
}

static bool x86_64_startSystemButtons(void *data) {
return false;
}

static int32_t x86_64_getDevkitVersion(void *data) {
	return 0;
}

platform_driver_t platform_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_main,
	x86_64_startSystemButtons,
	x86_64_getDevkitVersion,
};
