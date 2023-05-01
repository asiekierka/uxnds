#include <stdlib.h>

#include "../uxn.h"
#include "screen.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

UxnScreen uxn_screen;

/* c = !ch ? (color % 5 ? color >> 2 : 0) : color % 4 + ch == 1 ? 0 : (ch - 2 + (color & 3)) % 3 + 1; */

static Uint8 blending[4][16] = {
	{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
	{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
	{1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
	{2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2}};

static void
screen_fill(UxnScreen *s, Uint8 *pixels, Uint16 x1, Uint16 y1, Uint16 x2, Uint16 y2, Uint8 color)
{
	int x, y, width = s->width, height = s->height;
	for(y = y1; y < y2 && y < height; y++)
		for(x = x1; x < x2 && x < width; x++)
			pixels[x + y * width] = color;
}

static void
screen_blit(UxnScreen *s, Uint8 *pixels, Uint16 x1, Uint16 y1, Uint8 *ram, Uint16 addr, Uint8 color, Uint8 flipx, Uint8 flipy, Uint8 twobpp)
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
screen_palette(UxnScreen *p, Uint8 *addr)
{
	int i, shift;
	for(i = 0, shift = 4; i < 4; ++i, shift ^= 4) {
		Uint8
			r = (addr[0 + i / 2] >> shift) & 0xf,
			g = (addr[2 + i / 2] >> shift) & 0xf,
			b = (addr[4 + i / 2] >> shift) & 0xf;
		p->palette[i] = 0x0f000000 | r << 16 | g << 8 | b;
		p->palette[i] |= p->palette[i] << 4;
	}
	p->fg.changed = p->bg.changed = 1;
}

void
screen_resize(UxnScreen *p, Uint16 width, Uint16 height)
{
	Uint8 *bg, *fg;
	Uint32 *pixels;
	if(width < 0x8 || height < 0x8 || width >= 0x400 || height >= 0x400)
		return;
	bg = realloc(p->bg.pixels, width * height),
	fg = realloc(p->fg.pixels, width * height);
	pixels = realloc(p->pixels, width * height * sizeof(Uint32));
	if(!bg || !fg || !pixels)
		return;
	p->bg.pixels = bg;
	p->fg.pixels = fg;
	p->pixels = pixels;
	p->width = width;
	p->height = height;
	screen_fill(p, p->bg.pixels, 0, 0, p->width, p->height, 0);
	screen_fill(p, p->fg.pixels, 0, 0, p->width, p->height, 0);
}

void
screen_redraw(UxnScreen *p)
{
	Uint32 i, size = p->width * p->height, palette[16], *pixels = p->pixels;
	Uint8 *fg = p->fg.pixels, *bg = p->bg.pixels;
	for(i = 0; i < 16; i++)
		palette[i] = p->palette[(i >> 2) ? (i >> 2) : (i & 3)];
	for(i = 0; i < size; i++)
		pixels[i] = palette[fg[i] << 2 | bg[i]];
	p->fg.changed = p->bg.changed = 0;
}

Uint8
screen_dei(Uxn *u, Uint8 addr)
{
	switch(addr) {
	case 0x22: return uxn_screen.width >> 8;
	case 0x23: return uxn_screen.width;
	case 0x24: return uxn_screen.height >> 8;
	case 0x25: return uxn_screen.height;
	default: return u->dev[addr];
	}
}

void
screen_deo(Uint8 *ram, Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x3:
		screen_resize(&uxn_screen, PEEK2(d + 2), uxn_screen.height);
		break;
	case 0x5:
		screen_resize(&uxn_screen, uxn_screen.width, PEEK2(d + 4));
		break;
	case 0xe: {
		Uint8 ctrl = d[0xe];
		Uint8 color = ctrl & 0x3;
		Uint16 x = PEEK2(d + 0x8);
		Uint16 y = PEEK2(d + 0xa);
		Layer *layer = (ctrl & 0x40) ? &uxn_screen.fg : &uxn_screen.bg;
		/* fill mode */
		if(ctrl & 0x80) {
			Uint16 x2 = uxn_screen.width;
			Uint16 y2 = uxn_screen.height;
			if(ctrl & 0x10) x2 = x, x = 0;
			if(ctrl & 0x20) y2 = y, y = 0;
			screen_fill(&uxn_screen, layer->pixels, x, y, x2, y2, color);
			layer->changed = 1;
		}
		/* pixel mode */
		else {
			Uint16 width = uxn_screen.width;
			Uint16 height = uxn_screen.height;
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
		Layer *layer = (ctrl & 0x40) ? &uxn_screen.fg : &uxn_screen.bg;
		for(i = 0; i <= length; i++) {
			screen_blit(&uxn_screen, layer->pixels, x + dy * i, y + dx * i, ram, addr, ctrl & 0xf, ctrl & 0x10, ctrl & 0x20, twobpp);
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
