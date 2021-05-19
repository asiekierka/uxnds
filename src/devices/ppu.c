#include "ppu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static Uint8 font[][8] = {
	{0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c},
	{0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
	{0x00, 0x7c, 0x82, 0x02, 0x7c, 0x80, 0x80, 0xfe},
	{0x00, 0x7c, 0x82, 0x02, 0x1c, 0x02, 0x82, 0x7c},
	{0x00, 0x0c, 0x14, 0x24, 0x44, 0x84, 0xfe, 0x04},
	{0x00, 0xfe, 0x80, 0x80, 0x7c, 0x02, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x80, 0xfc, 0x82, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x02, 0x1e, 0x02, 0x02, 0x02},
	{0x00, 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x82, 0x7e, 0x02, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x02, 0x7e, 0x82, 0x82, 0x7e},
	{0x00, 0xfc, 0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc},
	{0x00, 0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c},
	{0x00, 0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc},
	{0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x82, 0x7c},
	{0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x80, 0x80}};

Uint8
readpixel(Uint8 *sprite, Uint8 h, Uint8 v)
{
	Uint8 ch1 = ((sprite[v] >> h) & 0x1);
	Uint8 ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
	return ch1 + ch2;
}

void
clear(Ppu *p)
{
	int i, sz = p->height * p->width;
	for(i = 0; i < sz; ++i) {
		p->fg.pixels[i] = p->fg.colors[0];
		p->bg.pixels[i] = p->bg.colors[0];
	}
}

void
putcolors(Ppu *p, Uint8 *addr)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = (*(addr + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			g = (*(addr + 2 + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			b = (*(addr + 4 + i / 2) >> (!(i % 2) << 2)) & 0x0f;
		p->bg.colors[i] = 0xff000000 | (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
		p->fg.colors[i] = 0xff000000 | (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
	}
	p->fg.colors[0] = 0;
	clear(p);
}

void
putpixel(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= p->width || y >= p->height)
		return;
	layer->pixels[y * p->width + x] = layer->colors[color];
}

void
puticn(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
			if(ch1 == 1 || (color != 0x05 && color != 0x0a && color != 0x0f))
				putpixel(p,
					layer,
					x + (flipx ? 7 - h : h),
					y + (flipy ? 7 - v : v),
					ch1 ? color % 4 : color / 4);
		}
}

void
putchr(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1) * color;
			Uint8 ch2 = ((sprite[v + 8] >> (7 - h)) & 0x1) * color;
			putpixel(p,
				layer,
				x + (flipx ? 7 - h : h),
				y + (flipy ? 7 - v : v),
				(((ch1 + ch2 * 2) + color / 4) & 0x3));
		}
}

/* output */

void
drawdebugger(Ppu *p, Uint8 *stack, Uint8 ptr)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x20; ++i) { /* memory */
		x = ((i % 8) * 3 + 1) * 8, y = (i / 8 + 1) * 8, b = stack[i];
		puticn(p, &p->bg, x, y, font[(b >> 4) & 0xf], 1 + (ptr == i) * 0x7, 0, 0);
		puticn(p, &p->bg, x + 8, y, font[b & 0xf], 1 + (ptr == i) * 0x7, 0, 0);
	}
	for(x = 0; x < 0x20; ++x) {
		putpixel(p, &p->bg, x, p->height / 2, 2);
		putpixel(p, &p->bg, p->width - x, p->height / 2, 2);
		putpixel(p, &p->bg, p->width / 2, p->height - x, 2);
		putpixel(p, &p->bg, p->width / 2, x, 2);
		putpixel(p, &p->bg, p->width / 2 - 16 + x, p->height / 2, 2);
		putpixel(p, &p->bg, p->width / 2, p->height / 2 - 16 + x, 2);
	}
}

int
initppu(Ppu *p, Uint8 hor, Uint8 ver)
{
	p->hor = hor;
	p->ver = ver;
	p->width = 8 * p->hor;
	p->height = 8 * p->ver;
	if(!(p->bg.pixels = malloc(p->width * p->height * sizeof(Uint32))))
		return 0;
	if(!(p->fg.pixels = malloc(p->width * p->height * sizeof(Uint32))))
		return 0;
	clear(p);
	return 1;
}
