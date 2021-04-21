/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "ppu.h"

static Uint8 font[][8] = {
	{0x00, 0x3c, 0x46, 0x4a, 0x52, 0x62, 0x3c, 0x00},
	{0x00, 0x18, 0x28, 0x08, 0x08, 0x08, 0x3e, 0x00},
	{0x00, 0x3c, 0x42, 0x02, 0x3c, 0x40, 0x7e, 0x00},
	{0x00, 0x3c, 0x42, 0x1c, 0x02, 0x42, 0x3c, 0x00},
	{0x00, 0x08, 0x18, 0x28, 0x48, 0x7e, 0x08, 0x00},
	{0x00, 0x7e, 0x40, 0x7c, 0x02, 0x42, 0x3c, 0x00},
	{0x00, 0x3c, 0x40, 0x7c, 0x42, 0x42, 0x3c, 0x00},
	{0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00},
	{0x00, 0x3c, 0x42, 0x3c, 0x42, 0x42, 0x3c, 0x00},
	{0x00, 0x3c, 0x42, 0x42, 0x3e, 0x02, 0x3c, 0x00},
	{0x00, 0x3c, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x00},
	{0x00, 0x7c, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00},
	{0x00, 0x3c, 0x42, 0x40, 0x40, 0x42, 0x3c, 0x00},
	{0x00, 0x78, 0x44, 0x42, 0x42, 0x44, 0x78, 0x00},
	{0x00, 0x7e, 0x40, 0x7c, 0x40, 0x40, 0x7e, 0x00},
	{0x00, 0x7e, 0x40, 0x40, 0x7c, 0x40, 0x40, 0x00}};

void
clear(Ppu *p)
{
	int i, sz = p->height * p->width;
	for(i = 0; i < sz; ++i) {
		p->output[i] = p->colors[0];
		p->fg[i] = 0;
		p->bg[i] = 0;
		p->fg[sz + i] = 0;
		p->bg[sz + i] = 0;
	}
}

void
drawpixel(Ppu *p, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= p->pad && x <= p->width - p->pad - 1 && y >= p->pad && y <= p->height - p->pad - 1)
		p->output[y * p->width + x] = p->colors[color];
}

void
putpixel(Ppu *p, Uint8 *layer, Uint16 x, Uint16 y, Uint8 color)
{
	Uint16 row = (y % 8) + ((x / 8 + y / 8 * p->hor) * 16), col = 7 - (x % 8);
	if(x >= p->hor * 8 || y >= p->ver * 8 || row > (p->hor * p->ver * 16) - 8)
		return;
	if(color == 0 || color == 2)
		layer[row] &= ~(1UL << col);
	else
		layer[row] |= 1UL << col;
	if(color == 0 || color == 1)
		layer[row + 8] &= ~(1UL << col);
	else
		layer[row + 8] |= 1UL << col;
}

void
puticn(Ppu *p, Uint8 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
			if(ch1 == 0 && (color == 0x05 || color == 0x0a || color == 0x0f))
				continue;
			putpixel(p, layer, x + h, y + v, ch1 ? color % 4 : color / 4);
		}
}

void
putchr(Ppu *p, Uint8 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1) * color;
			Uint8 ch2 = ((sprite[v + 8] >> (7 - h)) & 0x1) * color;
			putpixel(p, layer, x + h, y + v, (((ch1 + ch2 * 2) + color / 4) & 0x3));
		}
}

void
drawdebugger(Ppu *p, Uint8 *stack, Uint8 ptr)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x20; ++i) { /* memory */
		x = ((i % 8) * 3 + 1) * 8, y = (i / 8 + 1) * 8, b = stack[i];
		puticn(p, p->bg, x, y, font[(b >> 4) & 0xf], 2 + (ptr == i));
		puticn(p, p->bg, x + 8, y, font[b & 0xf], 2 + (ptr == i));
	}
	for(x = 0; x < 0x20; ++x) {
		drawpixel(p, x, p->height / 2, 2);
		drawpixel(p, p->width - x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height - x, 2);
		drawpixel(p, p->width / 2, x, 2);
		drawpixel(p, p->width / 2 - 16 + x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height / 2 - 16 + x, 2);
	}
}

void
putcolors(Ppu *p, Uint16 rr, Uint16 gg, Uint16 bb)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = ((rr >> (1 - i / 2) * 8) >> (!(i % 2) << 2)) & 0x0f,
			g = ((gg >> (1 - i / 2) * 8) >> (!(i % 2) << 2)) & 0x0f,
			b = ((bb >> (1 - i / 2) * 8) >> (!(i % 2) << 2)) & 0x0f;
		p->colors[i] = (r << 20) + (r << 16) + (g << 12) + (g << 8) + (b << 4) + b;
	}
}

void
drawppu(Ppu *p)
{
	Uint16 x, y;
	for(y = 0; y < p->ver; ++y)
		for(x = 0; x < p->hor; ++x) {
			Uint8 v, h;
			Uint16 key = (y * p->hor + x) * 16;
			for(v = 0; v < 8; v++)
				for(h = 0; h < 8; h++) {
					Uint8 *sprite = &p->fg[key];
					Uint8 ch1 = ((sprite[v] >> h) & 0x1);
					Uint8 ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
					if(ch1 + ch2 == 0) {
						sprite = &p->bg[key];
						ch1 = ((sprite[v] >> h) & 0x1);
						ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
					}
					drawpixel(p, x * 8 + p->pad + 7 - h, y * 8 + p->pad + v, ch1 + ch2);
				}
		}
}

int
initppu(Ppu *p, Uint8 hor, Uint8 ver, Uint8 pad)
{
	p->hor = hor;
	p->ver = ver;
	p->pad = pad;
	p->width = (8 * p->hor + p->pad * 2);
	p->height = (8 * p->ver + p->pad * 2);
	if(!(p->output = malloc(p->width * p->height * sizeof(Uint32))))
		return 0;
	if(!(p->bg = malloc(p->width * p->height * sizeof(Uint8) * 2)))
		return 0;
	if(!(p->fg = malloc(p->width * p->height * sizeof(Uint8) * 2)))
		return 0;
	clear(p);
	return 1;
}