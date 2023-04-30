#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <tex3ds.h>

#include "../uxn.h"
#include "../util.h"
#include "ctr_screen.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick
Copyright (c) 2023 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

UxnCtrScreen uxn_ctr_screen;

/* c = !ch ? (color % 5 ? color >> 2 : 0) : color % 4 + ch == 1 ? 0 : (ch - 2 + (color & 3)) % 3 + 1; */

static Uint8 blending[4][16] = {
	{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
	{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
	{1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
	{2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2}};

static void
screen_fill(UxnCtrScreen *s, Uint8 *pixels, Uint16 x1, Uint16 y1, Uint16 x2, Uint16 y2, Uint8 color)
{
	int x, y, width = s->width, height = s->height;
	for(y = y1; y < y2 && y < height; y++)
		for(x = x1; x < x2 && x < width; x++)
			pixels[x + y * width] = color;
}

static void
screen_blit(UxnCtrScreen *s, Uint8 *pixels, Uint16 x1, Uint16 y1, Uint8 *ram, Uint16 addr, Uint8 color, Uint8 flipx, Uint8 flipy, Uint8 twobpp)
{
	int v, h, width = s->width, height = s->height, opaque = (color % 5) || !color;
	for(v = 0; v < 8; v++) {
		Uint16 c = ram[(addr + v) & 0xffff] | (twobpp ? (ram[(addr + v + 8) & 0xffff] << 8) : 0);
		Uint16 y = y1 + (flipy ? 7 - v : v);
		for(h = 7; h >= 0; --h, c >>= 1) {
			Uint8 ch = (c & 1) | ((c >> 7) & 2);
			if(opaque || ch) {
				Uint16 x = x1 + (flipx ? 7 - h : h);
				if(x < width && y < height)
					pixels[x + y * width] = blending[ch][color];
			}
		}
	}
}

void
ctr_screen_palette(UxnCtrScreen *p, Uint8 *addr)
{
	int i, shift;
	for(i = 0, shift = 4; i < 4; ++i, shift ^= 4) {
		Uint8
			r = (addr[0 + i / 2] >> shift) & 0xf,
			g = (addr[2 + i / 2] >> shift) & 0xf,
			b = (addr[4 + i / 2] >> shift) & 0xf;
		Uint32 color = 0x0f | r << 24 | g << 16 | b << 8;
		color |= color << 4;
		if (color != p->bg.palette[i]) {
			p->bg.palette[i] = color;
			p->bg.changed = true;
			if (i > 0) p->fg.changed = true;
		}
	}
	p->fg.palette[0] = 0;
	for (i = 1; i < 4; i++) {
		p->fg.palette[i] = p->bg.palette[i];
	}
}

static void
ctr_screen_clear_layer(UxnCtrScreen *p, Layer *layer)
{
	memset(layer->pixels, 0, p->width * p->height);
	layer->changed = true;
}

void
ctr_screen_free(UxnCtrScreen *p)
{
	linearFree(p->bg.gpuPixels);
	linearFree(p->fg.gpuPixels);
	free(p->bg.pixels);
	free(p->fg.pixels);
	C3D_TexDelete(&p->bg.gpuTexture);
	C3D_TexDelete(&p->fg.gpuTexture);
}

void
ctr_screen_init(UxnCtrScreen *p, Uint16 width, Uint16 height)
{
	Uint16
		pitch = next_power_of_two(width),
		texHeight = next_power_of_two(height);
	Uint8
		*bg = malloc(width * height),
		*fg = malloc(width * height);
	Uint32
		*bgPixels = linearAlloc(pitch * height * sizeof(Uint32)),
		*fgPixels = linearAlloc(pitch * height * sizeof(Uint32));
	if(bg) p->bg.pixels = bg;
	if(fg) p->fg.pixels = fg;
	if(bgPixels) p->bg.gpuPixels = bgPixels;
	if(fgPixels) p->fg.gpuPixels = fgPixels;
	if(bg && fg && bgPixels && fgPixels) {
		p->width = width;
		p->height = height;
		p->pitch = pitch;
		ctr_screen_clear_layer(p, &p->bg);
		ctr_screen_clear_layer(p, &p->fg);

		C3D_TexInitVRAM(&p->bg.gpuTexture, p->pitch, texHeight, GPU_RGBA8);
		C3D_TexInitVRAM(&p->fg.gpuTexture, p->pitch, texHeight, GPU_RGBA8);

		p->bg.gpuImage.tex = &p->bg.gpuTexture;
		p->bg.gpuImage.subtex = &p->gpuSubTexture;
		p->fg.gpuImage.tex = &p->fg.gpuTexture;
		p->fg.gpuImage.subtex = &p->gpuSubTexture;

		p->gpuSubTexture.width = width;
		p->gpuSubTexture.height = height;
		p->gpuSubTexture.left = 0.0f;
		p->gpuSubTexture.top = 1.0f;
		p->gpuSubTexture.right = width / (float) pitch;
		p->gpuSubTexture.bottom = 1.0f - (height / (float) texHeight);
	}
}

static void
ctr_screen_redraw_layer(UxnCtrScreen *p, Layer *layer)
{
	Uint32 x, y, width = p->width, height = p->height, pitch_step = p->pitch - p->width, *dest = layer->gpuPixels;
	Uint8 *src = layer->pixels;

	GSPGPU_InvalidateDataCache(layer->gpuPixels, p->pitch * p->height * 4);
	for(y = 0; y < height; y++, dest += pitch_step) {
		for (x = 0; x < width; x++) {
			*(dest++) = layer->palette[*(src++)];
		}
	}
	GSPGPU_FlushDataCache(layer->gpuPixels, p->pitch * p->height * 4);

	C3D_SyncDisplayTransfer(
		layer->gpuPixels, GX_BUFFER_DIM(p->pitch, p->height),
		layer->gpuTexture.data, GX_BUFFER_DIM(p->pitch, p->height),
		(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) |
		GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO) |
		GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8))
	);

	layer->changed = false;
}

void
ctr_screen_redraw(UxnCtrScreen *p)
{
	if (p->bg.changed) ctr_screen_redraw_layer(p, &p->bg);
	if (p->fg.changed) ctr_screen_redraw_layer(p, &p->fg);
}

Uint8
ctr_screen_dei(Uxn *u, Uint8 addr)
{
	switch(addr) {
	case 0x22: return uxn_ctr_screen.width >> 8;
	case 0x23: return uxn_ctr_screen.width;
	case 0x24: return uxn_ctr_screen.height >> 8;
	case 0x25: return uxn_ctr_screen.height;
	default: return u->dev[addr];
	}
}

void
ctr_screen_deo(Uint8 *ram, Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x3:
		// ctr_screen_resize(&uxn_ctr_screen, PEEK2(d + 2), uxn_ctr_screen.height);
		ctr_screen_clear_layer(&uxn_ctr_screen, &uxn_ctr_screen.bg);
		ctr_screen_clear_layer(&uxn_ctr_screen, &uxn_ctr_screen.fg);
		break;
	case 0x5:
		// ctr_screen_resize(&uxn_ctr_screen, uxn_ctr_screen.width, PEEK2(d + 4));
		ctr_screen_clear_layer(&uxn_ctr_screen, &uxn_ctr_screen.bg);
		ctr_screen_clear_layer(&uxn_ctr_screen, &uxn_ctr_screen.fg);
		break;
	case 0xe: {
		Uint8 ctrl = d[0xe];
		Uint8 color = ctrl & 0x3;
		Uint16 x = PEEK2(d + 0x8);
		Uint16 y = PEEK2(d + 0xa);
		Layer *layer = (ctrl & 0x40) ? &uxn_ctr_screen.fg : &uxn_ctr_screen.bg;
		/* fill mode */
		if(ctrl & 0x80) {
			Uint16 x2 = uxn_ctr_screen.width;
			Uint16 y2 = uxn_ctr_screen.height;
			if(ctrl & 0x10) x2 = x, x = 0;
			if(ctrl & 0x20) y2 = y, y = 0;
			screen_fill(&uxn_ctr_screen, layer->pixels, x, y, x2, y2, color);
			layer->changed = 1;
		}
		/* pixel mode */
		else {
			Uint16 width = uxn_ctr_screen.width;
			Uint16 height = uxn_ctr_screen.height;
			if(x < width && y < height)
				layer->pixels[x + y * width] = color;
			layer->changed = 1;
			if(d[0x6] & 0x1) POKE2(d + 0x8, x + 1); /* auto x+1 */
			if(d[0x6] & 0x2) POKE2(d + 0xa, y + 1); /* auto y+1 */
		}
		break;
	}
	case 0xf: {
		Uint8 i;
		Uint8 ctrl = d[0xf];
		Uint8 move = d[0x6];
		Uint8 length = move >> 4;
		Uint8 twobpp = !!(ctrl & 0x80);
		Uint16 x = PEEK2(d + 0x8);
		Uint16 y = PEEK2(d + 0xa);
		Uint16 addr = PEEK2(d + 0xc);
		Uint16 dx = (move & 0x1) << 3;
		Uint16 dy = (move & 0x2) << 2;
		Layer *layer = (ctrl & 0x40) ? &uxn_ctr_screen.fg : &uxn_ctr_screen.bg;
		for(i = 0; i <= length; i++) {
			screen_blit(&uxn_ctr_screen, layer->pixels, x + dy * i, y + dx * i, ram, addr, ctrl & 0xf, ctrl & 0x10, ctrl & 0x20, twobpp);
			addr += (move & 0x04) << (1 + twobpp);
		}
		layer->changed = 1;
		if(move & 0x1) POKE2(d + 0x8, x + dx); /* auto x+8 */
		if(move & 0x2) POKE2(d + 0xa, y + dy); /* auto y+8 */
		if(move & 0x4) POKE2(d + 0xc, addr);   /* auto addr+length */
		break;
	}
	}
}
