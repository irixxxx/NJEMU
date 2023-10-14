/******************************************************************************

	video.c

	PS2ビデオ制御関数

******************************************************************************/

#include "emumain.h"

#include <stdlib.h>
#include <kernel.h>
#include <malloc.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>

#include <gsInline.h>


/******************************************************************************
	グローバル関数
******************************************************************************/

/* turn black GS Screen */
#define GS_BLACK GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x80)
/* Generic tint color */
#define GS_TEXT GS_SETREG_RGBA(0x80, 0x80, 0x80, 0x80)
/* Size of Persistent drawbuffer (Single Buffered) */
#define RENDER_QUEUE_PER_POOLSIZE 1024 * 256 // 256K of persistent renderqueue
/* Size of Oneshot drawbuffer (Double Buffered, so it uses this size * 2) */
#define RENDER_QUEUE_OS_POOLSIZE 1024 * 1024 * 2 // 2048K of oneshot renderqueue

typedef struct ps2_video {
	GSGLOBAL *gsGlobal;
	bool drawExtraInfo;

	// Original buffers containing clut indexes
	uint8_t *screen;
	uint8_t *spr;
	uint8_t *spr0;
	uint8_t *spr1;
	uint8_t *spr2;
	uint8_t *fix;


	GSTEXTURE *scrbitmap;
	GSTEXTURE *tex_spr0;
	GSTEXTURE *tex_spr1;
	GSTEXTURE *tex_spr2;
	GSTEXTURE *tex_fix;
	uint32_t offset;
	uint8_t vsync; /* 0 (Disabled), 1 (Enabled), 2 (Dynamic) */
	uint8_t pixel_format;
} ps2_video_t;

/*--------------------------------------------------------
	ビデオ処理初期化
--------------------------------------------------------*/
static GSTEXTURE *initializeTexture(int width, int height, void *mem) {
	GSTEXTURE *tex = (GSTEXTURE *)calloc(1, sizeof(GSTEXTURE));
	tex->Width = width;
	tex->Height = height;
	tex->PSM = GS_PSM_T8;
	tex->ClutPSM = GS_PSM_CT16;
	tex->Mem = mem;

	return tex;
}

static void ps2_start(void *data) {
	ps2_video_t *ps2 = (ps2_video_t*)data;

	GSGLOBAL *gsGlobal;
	
	gsGlobal = gsKit_init_global();

	gsGlobal->Mode = GS_MODE_NTSC;
    gsGlobal->Height = 448;

	gsGlobal->PSM  = GS_PSM_CT24;
	gsGlobal->PSMZ = GS_PSMZ_16S;
	gsGlobal->ZBuffering = GS_SETTING_OFF;
	gsGlobal->DoubleBuffering = GS_SETTING_ON;
	gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
	gsGlobal->Dithering = GS_SETTING_OFF;

	gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);

	dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);

	gsKit_set_clamp(gsGlobal, GS_CMODE_REPEAT);

	gsKit_vram_clear(gsGlobal);

	gsKit_init_screen(gsGlobal);

	gsKit_TexManager_init(gsGlobal);

	gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_clear(gsGlobal, GS_BLACK);
	ps2->gsGlobal = gsGlobal;

	// Original buffers containing clut indexes
	size_t scrbitmapSize = BUF_WIDTH * SCR_HEIGHT;
	size_t textureSize = BUF_WIDTH * TEXTURE_HEIGHT;
	ps2->screen = (uint8_t*)malloc(scrbitmapSize);
	uint8_t *spr = (uint8_t*)malloc(textureSize * 3);
	ps2->spr = spr;
	ps2->spr0 = spr;
	ps2->spr1 = spr + textureSize;
	ps2->spr2 = spr + textureSize * 2;
	ps2->fix = (uint8_t*)malloc(textureSize);

	// Initialize textures
	ps2->scrbitmap = initializeTexture(BUF_WIDTH, SCR_HEIGHT, ps2->screen);
	ps2->tex_spr0 = initializeTexture(BUF_WIDTH, TEXTURE_HEIGHT, ps2->spr0);
	ps2->tex_spr1 = initializeTexture(BUF_WIDTH, TEXTURE_HEIGHT, ps2->spr1);
	ps2->tex_spr2 = initializeTexture(BUF_WIDTH, TEXTURE_HEIGHT, ps2->spr2);
	ps2->tex_fix = initializeTexture(BUF_WIDTH, TEXTURE_HEIGHT, ps2->fix);

	ui_init();
}

static void *ps2_init(void)
{
	ps2_video_t *ps2 = (ps2_video_t*)calloc(1, sizeof(ps2_video_t));
	ps2->drawExtraInfo = true;

	ps2_start(ps2);
	return ps2;
}


/*--------------------------------------------------------
	ビデオ処理終了(共通)
--------------------------------------------------------*/

static void ps2_exit(ps2_video_t *ps2) {
	gsKit_clear(ps2->gsGlobal, GS_BLACK);
	gsKit_vram_clear(ps2->gsGlobal);
	gsKit_deinit_global(ps2->gsGlobal);
	
	free(ps2->scrbitmap);
	ps2->scrbitmap = NULL;
	free(ps2->tex_spr0);
	ps2->tex_spr0 = NULL;
	free(ps2->tex_spr1);
	ps2->tex_spr1 = NULL;
	free(ps2->tex_spr2);
	ps2->tex_spr2 = NULL;
	free(ps2->tex_fix);
	ps2->tex_fix = NULL;

	free(ps2->screen);
	ps2->screen = NULL;
	free(ps2->spr);
	ps2->spr = NULL;
	ps2->spr0 = NULL;
	ps2->spr1 = NULL;
	ps2->spr2 = NULL;
	free(ps2->fix);
	ps2->fix = NULL;
}

static void ps2_free(void *data)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;
	
	ps2_exit(ps2);
	free(ps2);
}

/*--------------------------------------------------------
	ビデオモード設定
--------------------------------------------------------*/


static void ps2_setMode(void *data, int mode)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;
#if VIDEO_32BPP
	if (video_mode != mode)
	{
		ps2_exit(ps2);
		video_mode = mode;

		ps2_start(data);
	}
#endif
}

/*--------------------------------------------------------
	VSYNCを待つ
--------------------------------------------------------*/

static void ps2_waitVsync(void *data)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;

	gsKit_sync_flip(ps2->gsGlobal);
}


/*--------------------------------------------------------
	スクリーンをフリップ
--------------------------------------------------------*/

static void ps2_flipScreen(void *data, bool vsync)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;

	gsKit_queue_exec(ps2->gsGlobal);
	gsKit_finish();

	if (vsync) gsKit_sync_flip(ps2->gsGlobal);

	gsKit_TexManager_nextFrame(ps2->gsGlobal);
    gsKit_clear(ps2->gsGlobal, GS_BLACK);
}


/*--------------------------------------------------------
	VRAMのアドレスを取得
--------------------------------------------------------*/

static void *ps2_frameAddr(void *data, void *frame, int x, int y)
{
	// TODO: FJTRUJY so far just used by the menu
// #if VIDEO_32BPP
// 	if (video_mode == 32)
// 		return (void *)(((uint32_t)frame | 0x44000000) + ((x + (y << 9)) << 2));
// 	else
// #endif
// 		return (void *)(((uint32_t)frame | 0x44000000) + ((x + (y << 9)) << 1));
	return NULL;
}

static void *ps2_workFrame(void *data, enum WorkBuffer buffer)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;
	switch (buffer)
	{
		case SCRBITMAP:
			return ps2->screen;
		case TEX_SPR0:
			return ps2->spr0;
		case TEX_SPR1:
			return ps2->spr1;
		case TEX_SPR2:
			return ps2->spr2;
		case TEX_FIX:
			return ps2->fix;
	}

	return NULL;
}


/*--------------------------------------------------------
	描画/表示フレームをクリア
--------------------------------------------------------*/

static void ps2_clearScreen(void *data)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;
	gsKit_clear(ps2->gsGlobal, GS_BLACK);
}

/*--------------------------------------------------------
	指定したフレームをクリア
--------------------------------------------------------*/

static void ps2_clearFrame(void *data, void *frame)
{
	ps2_clearScreen(data);
}


/*--------------------------------------------------------
	指定したフレームを塗りつぶし
--------------------------------------------------------*/

static void ps2_fillFrame(void *data, void *frame, uint32_t color)
{
	// TODO: FJTRUJY so far just used by the menu

	// sceGuStart(GU_DIRECT, gulist);
	// sceGuDrawBufferList(pixel_format, frame, BUF_WIDTH);
	// sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// sceGuClearColor(color);
	// sceGuClear(GU_COLOR_BUFFER_BIT | GU_FAST_CLEAR_BIT);
	// sceGuFinish();
	// sceGuSync(0, GU_SYNC_FINISH);
}


/*--------------------------------------------------------
	矩形範囲をコピー
--------------------------------------------------------*/

static void ps2_transferWorkFrame(void *data, RECT *src_rect, RECT *dst_rect)
{
	ps2_video_t *ps2 = (ps2_video_t*)data;
	if (!ps2->drawExtraInfo) return;
	gs_rgbaq color = color_to_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);

	// Choose texture to print
	GSTEXTURE *tex = ps2->tex_fix;

	#define LEFT 350
	#define TOP 20
	#define RIGHT (LEFT + tex->Width / 2)
	#define BOTTOM (TOP + tex->Height / 2)
	#define BORDER_LEFT LEFT - 1
	#define BORDER_TOP TOP - 1
	#define BORDER_RIGHT RIGHT + 1
	#define BORDER_BOTTOM BOTTOM + 1

	gsKit_prim_quad(ps2->gsGlobal, 
		BORDER_LEFT, BORDER_TOP, 
		BORDER_RIGHT, BORDER_TOP, 
		BORDER_LEFT, BORDER_BOTTOM, 
		BORDER_RIGHT, BORDER_BOTTOM, 
		0, GS_SETREG_RGBA(0x80, 0, 0, 0x80));
	gsKit_prim_quad(ps2->gsGlobal, 
		LEFT, TOP, 
		RIGHT, TOP, 
		LEFT, BOTTOM, 
		RIGHT, BOTTOM, 
		0, GS_SETREG_RGBA(0, 0, 0, 0x80));

	GSPRIMUVPOINT *verts2 = (GSPRIMUVPOINT *)malloc(sizeof(GSPRIMUVPOINT) * 2);
	verts2[0].xyz2 = vertex_to_XYZ2(ps2->gsGlobal, LEFT, TOP, 0);
	verts2[0].uv = vertex_to_UV(tex, 0, 0);
	verts2[0].rgbaq = color;

	verts2[1].xyz2 = vertex_to_XYZ2(ps2->gsGlobal, RIGHT, BOTTOM, 0);
	verts2[1].uv = vertex_to_UV(tex, tex->Width, tex->Height);
	verts2[1].rgbaq = color;

	gskit_prim_list_sprite_texture_uv_3d(ps2->gsGlobal, tex, 2, verts2);

	free(verts2);
}

static void ps2_copyRect(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// TODO: FJTRUJY so far just used by the menu, adhoc, and state
	// It is also used in the biosmenu but let's ignore it for now

	// int j, sw, dw, sh, dh;
	// struct Vertex *vertices;

	// sw = src_rect->right - src_rect->left;
	// dw = dst_rect->right - dst_rect->left;
	// sh = src_rect->bottom - src_rect->top;
	// dh = dst_rect->bottom - dst_rect->top;

	// sceGuStart(GU_DIRECT, gulist);

	// sceGuDrawBufferList(pixel_format, dst, BUF_WIDTH);
	// sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	// sceGuDisable(GU_ALPHA_TEST);

	// sceGuTexMode(pixel_format, 0, 0, GU_FALSE);
	// sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, GU_FRAME_ADDR(src));
	// if (sw == dw && sh == dh)
	// 	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	// else
	// 	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->left + j * dw / sw;
	// 	vertices[0].y = dst_rect->top;

	// 	vertices[1].u = src_rect->left + j + SLICE_SIZE;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
	// 	vertices[1].y = dst_rect->bottom;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// if (j < sw)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->left + j * dw / sw;
	// 	vertices[0].y = dst_rect->top;

	// 	vertices[1].u = src_rect->right;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->right;
	// 	vertices[1].y = dst_rect->bottom;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// sceGuFinish();
	// sceGuSync(0, GU_SYNC_FINISH);
}


/*--------------------------------------------------------
	矩形範囲を左右反転してコピー
--------------------------------------------------------*/

static void ps2_copyRectFlip(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// TODO: FJTRUJY not used so far in MVS


	// int16_t j, sw, dw, sh, dh;
	// struct Vertex *vertices;

	// sw = src_rect->right - src_rect->left;
	// dw = dst_rect->right - dst_rect->left;
	// sh = src_rect->bottom - src_rect->top;
	// dh = dst_rect->bottom - dst_rect->top;

	// sceGuStart(GU_DIRECT, gulist);

	// sceGuDrawBufferList(pixel_format, dst, BUF_WIDTH);
	// sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	// sceGuDisable(GU_ALPHA_TEST);

	// sceGuTexMode(pixel_format, 0, 0, GU_FALSE);
	// sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	// if (sw == dw && sh == dh)
	// 	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	// else
	// 	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	// {
    // 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->right - j * dw / sw;
	// 	vertices[0].y = dst_rect->bottom;

	// 	vertices[1].u = src_rect->left + j + SLICE_SIZE;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->right - (j + SLICE_SIZE) * dw / sw;
	// 	vertices[1].y = dst_rect->top;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// if (j < sw)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->right - j * dw / sw;
	// 	vertices[0].y = dst_rect->bottom;

	// 	vertices[1].u = src_rect->right;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->left;
	// 	vertices[1].y = dst_rect->top;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// sceGuFinish();
	// sceGuSync(0, GU_SYNC_FINISH);
}


/*--------------------------------------------------------
	矩形範囲を270度回転してコピー
--------------------------------------------------------*/

static void ps2_copyRectRotate(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// TODO: FJTRUJY not used so far in MVS (juat in state.c, but not used in the game)
	// int16_t j, sw, dw, sh, dh;
	// struct Vertex *vertices;

	// sw = src_rect->right - src_rect->left;
	// dw = dst_rect->right - dst_rect->left;
	// sh = src_rect->bottom - src_rect->top;
	// dh = dst_rect->bottom - dst_rect->top;

	// sceGuStart(GU_DIRECT, gulist);

	// sceGuDrawBufferList(pixel_format, dst, BUF_WIDTH);
	// sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	// sceGuDisable(GU_ALPHA_TEST);

	// sceGuTexMode(pixel_format, 0, 0, GU_FALSE);
	// sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	// if (sw == dh && sh == dw)
	// 	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	// else
	// 	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->right - j;
	// 	vertices[0].v = src_rect->bottom;
	// 	vertices[0].x = dst_rect->right;
	// 	vertices[0].y = dst_rect->top - j * dh / sw;

	// 	vertices[1].u = src_rect->right - j + SLICE_SIZE;
	// 	vertices[1].v = src_rect->top;
	// 	vertices[1].x = dst_rect->right;
	// 	vertices[1].y = dst_rect->bottom - (j + SLICE_SIZE) * dh / sw;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// if (j < sw)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->right + j;
	// 	vertices[0].v = src_rect->bottom;
	// 	vertices[0].x = dst_rect->right;
	// 	vertices[0].y = dst_rect->top - j * dh / sw;

	// 	vertices[1].u = src_rect->left;
	// 	vertices[1].v = src_rect->top;
	// 	vertices[1].x = dst_rect->left;
	// 	vertices[1].y = dst_rect->bottom;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// sceGuFinish();
	// sceGuSync(0, GU_SYNC_FINISH);
}


/*--------------------------------------------------------
	テクスチャを矩形範囲を指定して描画
--------------------------------------------------------*/

static void ps2_drawTexture(void *data, uint32_t src_fmt, uint32_t dst_fmt, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// TODO: FJTRUJY so far just used by the menu

	// int j, sw, dw, sh, dh;
	// struct Vertex *vertices;

	// sw = src_rect->right - src_rect->left;
	// dw = dst_rect->right - dst_rect->left;
	// sh = src_rect->bottom - src_rect->top;
	// dh = dst_rect->bottom - dst_rect->top;

	// sceGuStart(GU_DIRECT, gulist);
	// sceGuDrawBufferList(dst_fmt, dst, BUF_WIDTH);
	// sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);

	// sceGuTexMode(src_fmt, 0, 0, GU_FALSE);
	// sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, GU_FRAME_ADDR(src));
	// if (sw == dw && sh == dh)
	// 	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	// else
	// 	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	// {
    // 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->left + j * dw / sw;
	// 	vertices[0].y = dst_rect->top;

	// 	vertices[1].u = src_rect->left + j + SLICE_SIZE;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
	// 	vertices[1].y = dst_rect->bottom;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// if (j < sw)
	// {
	// 	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	// 	vertices[0].u = src_rect->left + j;
	// 	vertices[0].v = src_rect->top;
	// 	vertices[0].x = dst_rect->left + j * dw / sw;
	// 	vertices[0].y = dst_rect->top;

	// 	vertices[1].u = src_rect->right;
	// 	vertices[1].v = src_rect->bottom;
	// 	vertices[1].x = dst_rect->right;
	// 	vertices[1].y = dst_rect->bottom;

	// 	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, NULL, vertices);
	// }

	// sceGuFinish();
	// sceGuSync(0, GU_SYNC_FINISH);
}

static void *ps2_getNativeObjects(void *data, int index) {
	ps2_video_t *ps2 = (ps2_video_t*)data;
	switch ( index) {
	case 0:
		return ps2->gsGlobal;
	case 1: 
		return ps2->scrbitmap;
	case 2:
		return ps2->tex_spr0;
	case 3:
		return ps2->tex_spr1;
	case 4:
		return ps2->tex_spr2;
	case 5:
		return ps2->tex_fix;
	default:
		return NULL;
	}
}

static GSTEXTURE *ps2_getTexture(void *data, enum WorkBuffer buffer) {
	ps2_video_t *ps2 = (ps2_video_t*)data;
	switch (buffer)
	{
		case SCRBITMAP:
			return ps2->scrbitmap;
		case TEX_SPR0:
			return ps2->tex_spr0;
		case TEX_SPR1:
			return ps2->tex_spr1;
		case TEX_SPR2:
			return ps2->tex_spr2;
		case TEX_FIX:
			return ps2->tex_fix;
		default:
			return NULL;
	}
}

static void ps2_blitTexture(void *data, enum WorkBuffer buffer, void *clut, uint32_t vertices_count, void *vertices) {
	// printf("ps2_blitTexture buffer: %d, vertices_count: %d\n", buffer, vertices_count);
	ps2_video_t *ps2 = (ps2_video_t*)data;
	GSTEXTURE *tex = ps2_getTexture(data, buffer);
	tex->Clut = clut;

	gsKit_TexManager_invalidate(ps2->gsGlobal, tex);
	gsKit_TexManager_bind(ps2->gsGlobal, tex);

	gskit_prim_list_sprite_texture_uv_3d(ps2->gsGlobal, tex, vertices_count, vertices);
}


video_driver_t video_ps2 = {
	"ps2",
	ps2_init,
	ps2_free,
	ps2_setMode,
	ps2_waitVsync,
	ps2_flipScreen,
	ps2_frameAddr,
	ps2_workFrame,
	ps2_clearScreen,
	ps2_clearFrame,
	ps2_fillFrame,
	ps2_transferWorkFrame,
	ps2_copyRect,
	ps2_copyRectFlip,
	ps2_copyRectRotate,
	ps2_drawTexture,
	ps2_getNativeObjects,
	ps2_blitTexture,
};