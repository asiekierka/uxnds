#include <stdlib.h>

#include "../uxn.h"
#include "../util.h"
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

static Uint8 blending[5][16] = {
	{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
	{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
	{1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
	{2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2},
	{1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0}};

static Uint32 palette_mono[] = {
	0x0f000000, 0x0fffffff};

static void
screen_write(UxnScreen *p, Layer *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x < p->width && y < p->height) {
		Uint32 i = x + y * p->width;
		if(color != layer->pixels[i]) {
			layer->pixels[i] = color;
			layer->changed = 1;
		}
	}
}

static void
screen_blit(UxnScreen *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy, Uint8 twobpp)
{
	int v, h, opaque = blending[4][color];
	for(v = 0; v < 8; v++) {
		Uint16 c = sprite[v] | (twobpp ? sprite[v + 8] : 0) << 8;
		for(h = 7; h >= 0; --h, c >>= 1) {
			Uint8 ch = (c & 1) | ((c >> 7) & 2);
			if(opaque || ch)
				screen_write(p,
					layer,
					x + (flipx ? 7 - h : h),
					y + (flipy ? 7 - v : v),
					blending[ch][color]);
		}
	}
}

void
screen_palette(UxnScreen *p, Uint8 *addr)
{
	int i, shift;
	for(i = 0, shift = 4; i < 4; ++i, shift ^= 4) {
		Uint8
			r = (addr[0 + i / 2] >> shift) & 0x0f,
			g = (addr[2 + i / 2] >> shift) & 0x0f,
			b = (addr[4 + i / 2] >> shift) & 0x0f;
		p->palette[i] = 0x0f000000 | r << 16 | g << 8 | b;
		p->palette[i] |= p->palette[i] << 4;
	}
	p->fg.changed = p->bg.changed = 1;
}

void
screen_resize(UxnScreen *p, Uint16 width, Uint16 height)
{
	Uint8
		*bg = realloc(p->bg.pixels, width * height),
		*fg = realloc(p->fg.pixels, width * height);
	Uint32
		*pixels = realloc(p->pixels, width * height * sizeof(Uint32));
	if(bg) p->bg.pixels = bg;
	if(fg) p->fg.pixels = fg;
	if(pixels) p->pixels = pixels;
	if(bg && fg && pixels) {
		p->width = width;
		p->height = height;
		screen_clear(p, &p->bg);
		screen_clear(p, &p->fg);
	}
}

void
screen_clear(UxnScreen *p, Layer *layer)
{
	Uint32 i, size = p->width * p->height;
	for(i = 0; i < size; i++)
		layer->pixels[i] = 0x00;
	layer->changed = 1;
}

void
screen_redraw(UxnScreen *p, Uint32 *pixels)
{
	Uint32 i, size = p->width * p->height, palette[16];
	for(i = 0; i < 16; i++)
		palette[i] = p->palette[(i >> 2) ? (i >> 2) : (i & 3)];
	if(p->mono) {
		for(i = 0; i < size; i++)
			pixels[i] = palette_mono[(p->fg.pixels[i] ? p->fg.pixels[i] : p->bg.pixels[i]) & 0x1];
	} else {
		for(i = 0; i < size; i++)
			pixels[i] = palette[p->fg.pixels[i] << 2 | p->bg.pixels[i]];
	}
	p->fg.changed = p->bg.changed = 0;
}

void
screen_mono(UxnScreen *p, Uint32 *pixels)
{
	p->mono = !p->mono;
	screen_redraw(p, pixels);
}

/* IO */

Uint8
screen_dei(Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x2: return uxn_screen.width >> 8;
	case 0x3: return uxn_screen.width;
	case 0x4: return uxn_screen.height >> 8;
	case 0x5: return uxn_screen.height;
	default: return d[port];
	}
}

void
screen_deo(Uint8 *ram, Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x3:
		if(!FIXED_SIZE) {
			Uint16 w;
			PEKDEV(w, 0x2);
			screen_resize(&uxn_screen, clamp(w, 1, 1024), uxn_screen.height);
		}
		break;
	case 0x5:
		if(!FIXED_SIZE) {
			Uint16 h;
			PEKDEV(h, 0x4);
			screen_resize(&uxn_screen, uxn_screen.width, clamp(h, 1, 1024));
		}
		break;
	case 0xe: {
		Uint16 x, y;
		Uint8 layer = d[0xe] & 0x40;
		PEKDEV(x, 0x8);
		PEKDEV(y, 0xa);
		screen_write(&uxn_screen, layer ? &uxn_screen.fg : &uxn_screen.bg, x, y, d[0xe] & 0x3);
		if(d[0x6] & 0x01) POKDEV(0x8, x + 1); /* auto x+1 */
		if(d[0x6] & 0x02) POKDEV(0xa, y + 1); /* auto y+1 */
		break;
	}
	case 0xf: {
		Uint16 x, y, dx, dy, addr;
		Uint8 i, n, twobpp = !!(d[0xf] & 0x80);
		Layer *layer = (d[0xf] & 0x40) ? &uxn_screen.fg : &uxn_screen.bg;
		PEKDEV(x, 0x8);
		PEKDEV(y, 0xa);
		PEKDEV(addr, 0xc);
		n = d[0x6] >> 4;
		dx = (d[0x6] & 0x01) << 3;
		dy = (d[0x6] & 0x02) << 2;
		if(addr > 0x10000 - ((n + 1) << (3 + twobpp)))
			return;
		for(i = 0; i <= n; i++) {
			screen_blit(&uxn_screen, layer, x + dy * i, y + dx * i, &ram[addr], d[0xf] & 0xf, d[0xf] & 0x10, d[0xf] & 0x20, twobpp);
			addr += (d[0x6] & 0x04) << (1 + twobpp);
		}
		POKDEV(0xc, addr);   /* auto addr+length */
		POKDEV(0x8, x + dx); /* auto x+8 */
		POKDEV(0xa, y + dy); /* auto y+8 */
		break;
	}
	}
}
