#include "emumain.h"

typedef struct x86_64_platform {
} x86_64_platform_t;

static void *x86_64_init(void) {
	x86_64_platform_t *x86_64 = (x86_64_platform_t*)calloc(1, sizeof(x86_64_platform_t));

	return x86_64;
}

static void x86_64_free(void *data) {
	x86_64_platform_t *x86_64 = (x86_64_platform_t*)data;

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
