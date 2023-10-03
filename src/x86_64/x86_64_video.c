/******************************************************************************

	video.c

	PS2ビデオ制御関数

******************************************************************************/

#include "emumain.h"

#include <stdlib.h>
#include <SDL.h>

typedef struct x86_64_video {
	SDL_Window* window;
	SDL_Renderer* renderer;

	uint8_t *scrbitmap;
	uint8_t *tex_spr0;
	uint8_t *tex_spr1;
	uint8_t *tex_spr2;
	uint8_t *tex_fix;

	SDL_Texture *sdl_texture_scrbitmap;
	SDL_Texture *sdl_texture_tex_spr0;
	SDL_Texture *sdl_texture_tex_spr1;
	SDL_Texture *sdl_texture_tex_spr2;
	SDL_Texture *sdl_texture_tex_fix;
} x86_64_video_t;

/******************************************************************************
	グローバル関数
******************************************************************************/

static void x86_64_start(void *data) {
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;

	size_t textureSize = BUF_WIDTH * SCR_HEIGHT;
	x86_64->scrbitmap = (uint8_t*)malloc(textureSize);
	x86_64->tex_spr0 = (uint8_t*)malloc(textureSize);
	x86_64->tex_spr1 = (uint8_t*)malloc(textureSize);
	x86_64->tex_spr2 = (uint8_t*)malloc(textureSize);
	x86_64->tex_fix = (uint8_t*)malloc(textureSize);

	// Create SDL textures
	x86_64->sdl_texture_scrbitmap = SDL_CreateTexture(x86_64->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (x86_64->sdl_texture_scrbitmap == NULL) {
		printf("Could not create sdl_texture_scrbitmap: %s\n", SDL_GetError());
		return;
	}	
	x86_64->sdl_texture_tex_spr0 = SDL_CreateTexture(x86_64->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (x86_64->sdl_texture_tex_spr0 == NULL) {
		printf("Could not create sdl_texture_tex_spr0: %s\n", SDL_GetError());
		return;
	}
	x86_64->sdl_texture_tex_spr1 = SDL_CreateTexture(x86_64->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (x86_64->sdl_texture_tex_spr1 == NULL) {
		printf("Could not create sdl_texture_tex_spr1: %s\n", SDL_GetError());
		return;
	}

	x86_64->sdl_texture_tex_spr2 = SDL_CreateTexture(x86_64->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (x86_64->sdl_texture_tex_spr2 == NULL) {
		printf("Could not create sdl_texture_tex_spr2: %s\n", SDL_GetError());
		return;
	}

	x86_64->sdl_texture_tex_fix = SDL_CreateTexture(x86_64->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (x86_64->sdl_texture_tex_fix == NULL) {
		printf("Could not create sdl_texture_tex_fix: %s\n", SDL_GetError());
		return;
	}

	ui_init();
}

static void *x86_64_init(void)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)calloc(1, sizeof(x86_64_video_t));

	// Create a window (width, height, window title)
	char title[256];
	sprintf(title, "%s %s", APPNAME_STR, VERSION_STR);
    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_SHOWN);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		free(x86_64);
		return NULL;
	}

	x86_64->window = window;

	// Create a renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// Check that the renderer was successfully created
	if (renderer == NULL) {
		// In the case that the renderer could not be made...
		printf("Could not create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(x86_64->window);
		free(x86_64);
		return NULL;
	}

	x86_64->renderer = renderer;

	x86_64_start(x86_64);
	return x86_64;
}


/*--------------------------------------------------------
	ビデオ処理終了(共通)
--------------------------------------------------------*/

static void x86_64_exit(x86_64_video_t *x86_64) {
	if (x86_64->sdl_texture_scrbitmap) {
		SDL_DestroyTexture(x86_64->sdl_texture_scrbitmap);
		x86_64->sdl_texture_scrbitmap = NULL;
	}

	if (x86_64->sdl_texture_tex_spr0) {
		SDL_DestroyTexture(x86_64->sdl_texture_tex_spr0);
		x86_64->sdl_texture_tex_spr0 = NULL;
	}

	if (x86_64->sdl_texture_tex_spr1) {
		SDL_DestroyTexture(x86_64->sdl_texture_tex_spr1);
		x86_64->sdl_texture_tex_spr1 = NULL;
	}

	if (x86_64->sdl_texture_tex_spr2) {
		SDL_DestroyTexture(x86_64->sdl_texture_tex_spr2);
		x86_64->sdl_texture_tex_spr2 = NULL;
	}

	if (x86_64->sdl_texture_tex_fix) {
		SDL_DestroyTexture(x86_64->sdl_texture_tex_fix);
		x86_64->sdl_texture_tex_fix = NULL;
	}

	if (x86_64->scrbitmap) {
		free(x86_64->scrbitmap);
		x86_64->scrbitmap = NULL;
	}

	if (x86_64->tex_spr0) {
		free(x86_64->tex_spr0);
		x86_64->tex_spr0 = NULL;
	}

	if (x86_64->tex_spr1) {
		free(x86_64->tex_spr1);
		x86_64->tex_spr1 = NULL;
	}

	if (x86_64->tex_spr2) {
		free(x86_64->tex_spr2);
		x86_64->tex_spr2 = NULL;
	}

	if (x86_64->tex_fix) {
		free(x86_64->tex_fix);
		x86_64->tex_fix = NULL;
	}
}

static void x86_64_free(void *data)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;

	SDL_DestroyRenderer(x86_64->renderer);
	SDL_DestroyWindow(x86_64->window);
	
	x86_64_exit(x86_64);
	free(x86_64);
}

/*--------------------------------------------------------
	ビデオモード設定
--------------------------------------------------------*/


static void x86_64_setMode(void *data, int mode)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;
#if VIDEO_32BPP
	if (video_mode != mode)
	{
		x86_64_exit(x86_64);
		video_mode = mode;

		x86_64_start(data);
	}
#endif
}

/*--------------------------------------------------------
	VSYNCを待つ
--------------------------------------------------------*/

static void x86_64_waitVsync(void *data)
{
}


/*--------------------------------------------------------
	スクリーンをフリップ
--------------------------------------------------------*/

static void x86_64_flipScreen(void *data, bool vsync)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;
	SDL_RenderPresent(x86_64->renderer);
}


/*--------------------------------------------------------
	VRAMのアドレスを取得
--------------------------------------------------------*/

static void *x86_64_frameAddr(void *data, void *frame, int x, int y)
{
	return NULL;
}

static void *x86_64_workFrame(void *data, enum WorkBuffer buffer)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;
	switch (buffer) {
		case SCRBITMAP:
			return x86_64->scrbitmap;
		case TEX_SPR0:
			return x86_64->tex_spr0;
		case TEX_SPR1:
			return x86_64->tex_spr1;
		case TEX_SPR2:
			return x86_64->tex_spr2;
		case TEX_FIX:
			return x86_64->tex_fix;
		default:
			return NULL;
	}
}


/*--------------------------------------------------------
	描画/表示フレームをクリア
--------------------------------------------------------*/

static void x86_64_clearScreen(void *data)
{
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;
	SDL_RenderClear(x86_64->renderer);
}

/*--------------------------------------------------------
	指定したフレームをクリア
--------------------------------------------------------*/

static void x86_64_clearFrame(void *data, void *frame)
{
}


/*--------------------------------------------------------
	指定したフレームを塗りつぶし
--------------------------------------------------------*/

static void x86_64_fillFrame(void *data, void *frame, uint32_t color)
{
}


/*--------------------------------------------------------
	矩形範囲をコピー
--------------------------------------------------------*/

static void x86_64_transferWorkFrame(void *data, RECT *src_rect, RECT *dst_rect)
{
}

static void x86_64_copyRect(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
}


/*--------------------------------------------------------
	矩形範囲を左右反転してコピー
--------------------------------------------------------*/

static void x86_64_copyRectFlip(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
}


/*--------------------------------------------------------
	矩形範囲を270度回転してコピー
--------------------------------------------------------*/

static void x86_64_copyRectRotate(void *data, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
}


/*--------------------------------------------------------
	テクスチャを矩形範囲を指定して描画
--------------------------------------------------------*/

static void x86_64_drawTexture(void *data, uint32_t src_fmt, uint32_t dst_fmt, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
}

static void *x86_64_getNativeObjects(void *data, int index) {
	return NULL;
}

static SDL_Texture *x86_64_getTexture(void *data, enum WorkBuffer buffer) {
	x86_64_video_t *x86_64 = (x86_64_video_t*)data;
	switch (buffer) {
		case SCRBITMAP:
			return x86_64->sdl_texture_scrbitmap;
		case TEX_SPR0:
			return x86_64->sdl_texture_tex_spr0;
		case TEX_SPR1:
			return x86_64->sdl_texture_tex_spr1;
		case TEX_SPR2:
			return x86_64->sdl_texture_tex_spr2;
		case TEX_FIX:
			return x86_64->sdl_texture_tex_fix;
		default:
			return NULL;
	}
}

static void x86_64_blitFinishFix(void *data, enum WorkBuffer buffer, void *clut, uint32_t vertices_count, void *vertices) {
	// We need to transform the texutres saved that uses clut into a SDL texture compatible format
	x86_64_video_t *x86_64 = (x86_64_video_t *)data;
	SDL_Vertex *vertexs = (SDL_Vertex *)vertices;
	uint16_t *clut_texture = (uint16_t *)clut;
	uint8_t *tex_fix = x86_64_workFrame(data, buffer);
	SDL_Texture *texture = x86_64_getTexture(data, buffer);

	// print whole clut_texture content in hex
	printf("****************** clut_texture:\n");
	for (int i = 0; i < 256; ++i) {
		if (i % 16 == 0 && i != 0) {
			printf("\n");
		}
		printf("0x%04x ", clut_texture[i]);
	}
	printf("\n *********************** \n");

	// Lock texture
	void *pixels;
	int pitch;
	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	// Obtain the color from the clut using the index and copy it to the pixels array
	for (int i = 0; i < SCR_HEIGHT; ++i) {
		for (int j = 0; j < BUF_WIDTH; ++j) {
			int index = i * BUF_WIDTH + j;
			uint8_t pixelValue = tex_fix[index];
			uint16_t color = clut_texture[pixelValue];

			uint16_t *pixel = (uint16_t*)pixels + index;
			*pixel = color;
		}
	}

	// Unlock texture
	SDL_UnlockTexture(texture);

	// Render Geomertry
	SDL_RenderGeometry(x86_64->renderer, texture, vertexs, vertices_count, NULL, 0);
}


video_driver_t video_x86_64 = {
	"x86_64",
	x86_64_init,
	x86_64_free,
	x86_64_setMode,
	x86_64_waitVsync,
	x86_64_flipScreen,
	x86_64_frameAddr,
	x86_64_workFrame,
	x86_64_clearScreen,
	x86_64_clearFrame,
	x86_64_fillFrame,
	x86_64_transferWorkFrame,
	x86_64_copyRect,
	x86_64_copyRectFlip,
	x86_64_copyRectRotate,
	x86_64_drawTexture,
	x86_64_getNativeObjects,
	x86_64_blitFinishFix,
};