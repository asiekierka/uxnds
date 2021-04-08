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

Uint8 font[][8] = {
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
drawchr(Ppu *p, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 alpha)
{
	Uint8 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> h) & 0x1);
			Uint8 ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
			if(!alpha || (alpha && ch1 + ch2 != 0))
				drawpixel(p, x + 7 - h, y + v, ch1 + ch2);
		}
}

void
drawicn(Ppu *p, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 fg, Uint8 bg)
{
	Uint8 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = (sprite[v] >> (7 - h)) & 0x1;
			drawpixel(p, x + h, y + v, ch1 ? fg : bg);
		}
}

void
drawdebugger(Ppu *p, Uint8 *stack, Uint8 ptr)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x10; ++i) { /* memory */
		x = ((i % 8) * 3 + 3) * 8, y = p->pad + 8 + i / 8 * 8, b = stack[i];
		drawicn(p, x, y, font[(b >> 4) & 0xf], 1 + (ptr == i), 0);
		drawicn(p, x + 8, y, font[b & 0xf], 1 + (ptr == i), 0);
	}
	for(x = 0; x < 32; ++x) {
		drawpixel(p, x, p->height / 2, 2);
		drawpixel(p, p->width - x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height - x, 2);
		drawpixel(p, p->width / 2, x, 2);
		drawpixel(p, p->width / 2 - 16 + x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height / 2 - 16 + x, 2);
	}
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
loadtheme(Ppu *p, Uint8 *addr)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = (*(addr + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			g = (*(addr + 2 + i / 2) >> (!(i % 2) << 2)) & 0x0f,
			b = (*(addr + 4 + i / 2) >> (!(i % 2) << 2)) & 0x0f;
		p->colors[i] = (r << 20) + (r << 16) + (g << 12) + (g << 8) + (b << 4) + b;
	}
	p->reqdraw = 1;
}

void
drawppu(Ppu *p)
{
	Uint16 x, y;
	for(y = 0; y < p->ver; ++y)
		for(x = 0; x < p->hor; ++x) {
			Uint16 key = (y * p->hor + x) * 16;
			drawchr(p, x * 8 + p->pad, y * 8 + p->pad, &p->bg[key], 0);
			drawchr(p, x * 8 + p->pad, y * 8 + p->pad, &p->fg[key], 1);
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