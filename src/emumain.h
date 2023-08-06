/******************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#ifndef EMUMAIN_H
#define EMUMAIN_H

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

enum
{
	LOOP_EXIT = 0,
	LOOP_BROWSER,
	LOOP_RESTART,
	LOOP_RESET,
	LOOP_EXEC
};

enum
{
	UI_PAL_TITLE = 0,
	UI_PAL_SELECT,
	UI_PAL_NORMAL,
	UI_PAL_INFO,
	UI_PAL_WARNING,
	UI_PAL_BG1,
	UI_PAL_BG2,
	UI_PAL_FRAME,
	UI_PAL_FILESEL1,
	UI_PAL_FILESEL2,
	UI_PAL_MAX
};

enum
{
	WP_LOGO = 0,
	WP_FILER,
	WP_GAMECFG,
	WP_KEYCFG,
	WP_STATE,
	WP_COLORCFG,
	WP_DIPSW,
	WP_CMDLIST,
	NUM_WALLPAPERS
};

enum
{
	ICON_CONFIG = 0,
	ICON_KEYCONFIG,
	ICON_FOLDER,
	ICON_SYSTEM,
	ICON_RETURN,
	ICON_EXIT,
	ICON_DIPSWITCH,
	ICON_CMDLIST,
	ICON_UPPERDIR,
	ICON_MEMSTICK,
	ICON_ZIPFILE,
	ICON_BATTERY1,
	ICON_BATTERY2,
	ICON_BATTERY3,
	ICON_BATTERY4,
	ICON_COMMANDDAT,
	ICON_COLORCFG,
	MAX_ICONS
};

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>

#if defined(PSP)
#include "psp/psp.h"
#endif

#include "include/cpuintrf.h"
#include "include/memory.h"
#include "zip/zfile.h"
#include "common/loadrom.h"
#include "common/state.h"
#include "common/sound.h"
#include "common/power_driver.h"
#include "common/ticker_driver.h"
#include "common/platform_driver.h"
#include "common/video_driver.h"
#include "common/ui_text_driver.h"
#include "common/input_driver.h"
#ifdef ADHOC
#include "common/adhoc.h"
#endif
#if USE_CACHE
#include "common/cache.h"
#endif
#ifdef COMMAND_LIST
#include "common/cmdlist.h"
#endif
#ifdef CHEAT
#include "common/cheat.h"
#endif
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "common/coin.h"
#endif
#if (EMU_SYSTEM == NCDZ)
#include "common/mp3.h"
#endif

#if (EMU_SYSTEM == CPS1)
#include "cps1/cps1.h"
#elif (EMU_SYSTEM == CPS2)
#include "cps2/cps2.h"
#elif (EMU_SYSTEM == MVS)
#include "mvs/mvs.h"
#elif (EMU_SYSTEM == NCDZ)
#include "ncdz/ncdz.h"
#endif

extern char launchDir[MAX_PATH];
extern char screenshotDir[MAX_PATH];
extern bool systembuttons_available;

extern char game_name[16];
extern char parent_name[16];
extern char game_dir[MAX_PATH];

#if USE_CACHE
extern char cache_parent_name[16];
extern char cache_dir[MAX_PATH];
#endif

extern int option_showfps;
extern int option_autoframeskip;
extern int option_frameskip;
extern int option_speedlimit;
extern int option_vsync;
extern int option_stretch;

extern int option_sound_enable;
extern int option_samplerate;
extern int option_sound_volume;

extern int machine_driver_type;
extern int machine_input_type;
extern int machine_init_type;
extern int machine_screen_type;
extern int machine_sound_type;

extern uint32_t frames_displayed;
extern int fatal_error;

extern volatile int Loop;
extern volatile int Sleep;

void emu_main(void);

void autoframeskip_reset(void);

uint8_t skip_this_frame(void);
void update_screen(void);

void fatalerror(const char *text, ...);
void show_fatal_error(void);

void save_snapshot(void);

void waitVBlank(void);

#endif /* EMUMAIN_H */
