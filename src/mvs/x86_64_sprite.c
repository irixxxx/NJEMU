/******************************************************************************

	sprite.c

	MVS ¥¹¥×¥é¥¤¥È¥Þ¥Í©`¥¸¥ã

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	¶¨Êý/¥Þ¥¯¥íµÈ
******************************************************************************/

#define TEXTURE_HEIGHT	512

#define MAKE_FIX_KEY(code, attr)	(code | (attr << 28))
#define MAKE_SPR_KEY(code, attr)	(code | ((attr & 0x0f00) << 20))
#define PSP_UNCACHE_PTR(p)			(((uint32_t)(p)) | 0x40000000)


void blit_clear_all_sprite(void) {}

void blit_reset(void) {}
void blit_start(int start, int end) {}
void blit_finish(void) {}

void blit_update_object(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_draw_object(int16_t x, int16_t y, uint16_t z, int16_t priority, uint32_t code, uint16_t attr) {}

void blit_update_scroll1(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_draw_scroll1(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_finish_scroll1(void) {}

void blit_set_clip_scroll2(int16_t min_y, int16_t max_y) {}
int blit_check_clip_scroll2(int16_t sy) { return 0; }
void blit_update_scroll2(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_finish_scroll2(void) {}

void blit_update_scroll3(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_draw_scroll3(int16_t x, int16_t y, uint32_t code, uint16_t attr) {}
void blit_finish_scroll3(void) {}


void blit_set_fix_clear_flag(void) {}
void blit_set_spr_clear_flag(void) {}
void blit_draw_fix(int x, int y, uint32_t code, uint16_t attr) {}
void blit_finish_fix(void) {}
void blit_draw_spr_line(int x, int y, int zoom_x, int sprite_y, uint32_t code, uint16_t attr, uint8_t opaque) {}
void blit_finish_spr(void) {}
void blit_draw_spr(int x, int y, int w, int h, uint32_t code, uint16_t attr) {}