#include <nds.h>
#include "../../include/uxn.h"
#include "ppu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick
Copyright (c) 2021 Adrian "asie" Siekierka

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

DTCM_BSS
static Uint32 tile_dirty[24];

void
putcolors(Ppu *p, Uint8 *addr)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = (*(addr + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f,
			g = (*(addr + 2 + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f,
			b = (*(addr + 4 + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f;
		BG_PALETTE[i] = RGB15(
			(r << 1) | (r >> 3),
			(g << 1) | (g >> 3),
			(b << 1) | (b >> 3)
		);
	}
}

ITCM_ARM_CODE
void
putpixel(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= 32 * 8 || y >= 24 * 8)
		return;
	Uint32 pos = ((y & 7) + (((x >> 3) + (y >> 3) * 32) * 8));
	Uint32 shift = (x & 7) << 2;
	layer[pos] = (layer[pos] & (~(0xF << shift))) | (color << shift);
	tile_dirty[y >> 3] |= 1 << (x >> 3);
}

ITCM_ARM_CODE
void
puticn(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
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
					ch1 ? (color & 3) : (color >> 2));
		}
}

ITCM_ARM_CODE
void
putchr(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
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
				((((ch1 + ch2) << 1) + (color >> 2)) & 0x3));
		}
}

/* output */

/* void
drawdebugger(Ppu *p, Uint8 *stack, Uint8 ptr)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x20; ++i) { // memory
		x = ((i % 8) * 3 + 1) * 8, y = (i / 8 + 1) * 8, b = stack[i];
		puticn(p, p->bg, x, y, font[(b >> 4) & 0xf], 1 + (ptr == i) * 0x7, 0, 0);
		puticn(p, p->bg, x + 8, y, font[b & 0xf], 1 + (ptr == i) * 0x7, 0, 0);
	}
	for(x = 0; x < 0x20; ++x) {
		drawpixel(p, x, p->height / 2, 2);
		drawpixel(p, p->width - x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height - x, 2);
		drawpixel(p, p->width / 2, x, 2);
		drawpixel(p, p->width / 2 - 16 + x, p->height / 2, 2);
		drawpixel(p, p->width / 2, p->height / 2 - 16 + x, 2);
	}
} */

DTCM_BSS
static Uint32 tile_backup[8];

static inline void
copytile(Uint32 *tptr)
{
	// TODO: use ldmia/stmia for faster performance

	tile_backup[0] = tptr[0];
	tile_backup[1] = tptr[1];
	tile_backup[2] = tptr[2];
	tile_backup[3] = tptr[3];
	tile_backup[4] = tptr[4];
	tile_backup[5] = tptr[5];
	tile_backup[6] = tptr[6];
	tile_backup[7] = tptr[7];

	tptr = (Uint32*) (((u32) tptr) & 0xFFFEFFFF);

	tptr[0] = tile_backup[0];
	tptr[1] = tile_backup[1];
	tptr[2] = tile_backup[2];
	tptr[3] = tile_backup[3];
	tptr[4] = tile_backup[4];
	tptr[5] = tile_backup[5];
	tptr[6] = tile_backup[6];
	tptr[7] = tile_backup[7];
}

ITCM_ARM_CODE
void
copyppu(Ppu *p)
{
	int i, j, k, ofs;

	for (i = 0; i < 24; i++) {
		if (tile_dirty[i] != 0) {
			ofs = i << 8;
			k = 1;
			for (j = 0; j < 32; j++, ofs += 8, k <<= 1) {
				if (tile_dirty[i] & k) {
					copytile(p->bg + ofs);
					copytile(p->fg + ofs);
				}
			}
			tile_dirty[i] = 0;
		}
	}
}

int
initppu(Ppu *p, Uint8 hor, Uint8 ver)
{
	int i;
	u16 *map_ptr;

	p->hor = hor;
	p->ver = ver;
	p->width = (8 * p->hor);
	p->height = (8 * p->ver);

	videoSetMode(DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE | MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);

	// clear tile data
	p->bg = (Uint32*) BG_TILE_RAM(4);
	p->fg = (Uint32*) BG_TILE_RAM(6);
	for (i = 0; i < 8; i += 2) {
		dmaFillWords(0, BG_TILE_RAM(i), 768 * 32);
	}
	memset(tile_dirty, 0, sizeof(tile_dirty));

	// init bg data
	map_ptr = BG_GFX + (24576 >> 1);
	for (i = 0; i < 768; i++) {
		*(map_ptr++) = i;
	}

	REG_BG0CNT = BG_32x32 | BG_COLOR_16 | BG_PRIORITY_3 | BG_TILE_BASE(0) | BG_MAP_BASE(12);
	REG_BG1CNT = BG_32x32 | BG_COLOR_16 | BG_PRIORITY_2 | BG_TILE_BASE(2) | BG_MAP_BASE(12);

	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG1VOFS = 0;

	return 1;
}
