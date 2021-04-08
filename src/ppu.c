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
	int v, h;
	for(v = 0; v < HEIGHT; v++)
		for(h = 0; h < WIDTH; h++)
			p->output[v * WIDTH + h] = p->colors[0];
}

void
drawpixel(Ppu *p, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= p->x1 && x <= p->x2 && y >= p->x1 && y <= p->y2)
		p->output[y * WIDTH + x] = p->colors[color];
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
		x = ((i % 8) * 3 + 3) * 8, y = p->x1 + 8 + i / 8 * 8, b = stack[i];
		drawicn(p, x, y, font[(b >> 4) & 0xf], 1 + (ptr == i), 0);
		drawicn(p, x + 8, y, font[b & 0xf], 1 + (ptr == i), 0);
	}
	for(x = 0; x < 32; ++x) {
		drawpixel(p, x, HEIGHT / 2, 2);
		drawpixel(p, WIDTH - x, HEIGHT / 2, 2);
		drawpixel(p, WIDTH / 2, HEIGHT - x, 2);
		drawpixel(p, WIDTH / 2, x, 2);
		drawpixel(p, WIDTH / 2 - 16 + x, HEIGHT / 2, 2);
		drawpixel(p, WIDTH / 2, HEIGHT / 2 - 16 + x, 2);
	}
}

void
paintpixel(Uint8 *dst, Uint16 x, Uint16 y, Uint8 color)
{
	Uint16 row = (y % 8) + ((x / 8 + y / 8 * HOR) * 16), col = 7 - (x % 8);
	if(x >= HOR * 8 || y >= VER * 8 || row > RES - 8)
		return;
	if(color == 0 || color == 2)
		dst[row] &= ~(1UL << col);
	else
		dst[row] |= 1UL << col;
	if(color == 0 || color == 1)
		dst[row + 8] &= ~(1UL << col);
	else
		dst[row + 8] |= 1UL << col;
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

int
initppu(Ppu *p)
{
	if(!(p->output = (Uint32 *)malloc(WIDTH * HEIGHT * sizeof(Uint32))))
		return 0;
	clear(p);
	p->x1 = PAD * 8;
	p->x2 = WIDTH - PAD * 8 - 1;
	p->y1 = PAD * 8;
	p->y2 = HEIGHT - PAD * 8 - 1;
	return 1;
}