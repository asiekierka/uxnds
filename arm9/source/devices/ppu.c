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

/* static Uint8 font[][8] = {
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
	{0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x80, 0x80}}; */

DTCM_DATA
static Uint8 blending[5][16] = {
        {0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
        {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
        {1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
        {2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2},
        {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0}};

#define PPU_TILES_WIDTH 32
#define PPU_TILES_HEIGHT 24

DTCM_BSS
static Uint32 tile_dirty[PPU_TILES_HEIGHT + 1];

DTCM_DATA
static Uint32 lut_expand_8_32[256] = {
#include "lut_expand_8_32.inc"
};

DTCM_DATA
static Uint32 lut_expand_8_32_flipx[256] = {
#include "lut_expand_8_32_flipx.inc"
};

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
ppu_pixel(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= PPU_TILES_WIDTH * 8 || y >= PPU_TILES_HEIGHT * 8)
		return;
	Uint32 pos = ((y & 7) + (((x >> 3) + (y >> 3) * PPU_TILES_WIDTH) * 8));
	Uint32 shift = (x & 7) << 2;
	layer[pos] = (layer[pos] & (~(0xF << shift))) | (color << shift);
	tile_dirty[y >> 3] |= 1 << (x >> 3);
}

ITCM_ARM_CODE
void
ppu_1bpp(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint8 sprline;
	Uint8 xrightedge = x < ((PPU_TILES_WIDTH - 1) * 8);
	Uint16 v;
	Uint32 dirtyflag = (1 << (x >> 3)) | (1 << ((x + 7) >> 3));

	Uint32 layerpos = ((y & 7) + (((x >> 3) + (y >> 3) * PPU_TILES_WIDTH) * 8));
	Uint32 *layerptr = &layer[layerpos];
	Uint32 shift = (x & 7) << 2;
	Uint32 *lut_expand = flipx ? lut_expand_8_32 : lut_expand_8_32_flipx;

	if (flipy) flipy = 7;

	if(x >= PPU_TILES_WIDTH * 8 || y >= PPU_TILES_HEIGHT * 8)
		return;

	if (blending[4][color]) {
		u64 mask = ~((u64)0xFFFFFFFF << shift);

		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) >= (PPU_TILES_HEIGHT * 8)) break;

			sprline = sprite[v ^ flipy];
			u64 data = (u64)(lut_expand[sprline] * (color & 3)) << shift;
			data |= (u64)(lut_expand[sprline ^ 0xFF] * (color >> 2)) << shift;

			layerptr[0] = (layerptr[0] & mask) | data;
			if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else {
		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) >= (PPU_TILES_HEIGHT * 8)) break;

			sprline = sprite[v ^ flipy];
			u64 mask = ~((u64)(lut_expand[sprline] * 0xF) << shift);
			u64 data = (u64)(lut_expand[sprline] * (color & 3)) << shift;

			layerptr[0] = (layerptr[0] & mask) | data;
			if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	}

	tile_dirty[y >> 3] |= dirtyflag;
	tile_dirty[(y + 7) >> 3] |= dirtyflag;
}

#ifndef DEBUG
ITCM_ARM_CODE
#endif
void
ppu_2bpp(Ppu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint8 sprline1, sprline2;
	Uint8 xrightedge = x < ((PPU_TILES_WIDTH - 1) * 8);
	Uint16 v, h;
	Uint32 dirtyflag = (1 << (x >> 3)) | (1 << ((x + 7) >> 3));

	Uint32 layerpos = ((y & 7) + (((x >> 3) + (y >> 3) * PPU_TILES_WIDTH) * 8));
	Uint32 *layerptr = &layer[layerpos];
	Uint32 shift = (x & 7) << 2;

	if (flipy) flipy = 7;

	if(x >= PPU_TILES_WIDTH * 8 || y >= PPU_TILES_HEIGHT * 8)
		return;

	if (color == 1) {
		Uint32 *lut_expand = flipx ? lut_expand_8_32 : lut_expand_8_32_flipx;
		u64 mask = ~((u64)0xFFFFFFFF << shift);

		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) >= (PPU_TILES_HEIGHT * 8)) break;

			sprline1 = sprite[v ^ flipy];
			sprline2 = sprite[(v ^ flipy) | 8];

			u32 data32 = (lut_expand[sprline1]) | (lut_expand[sprline2] << 1);
			u64 data = ((u64) (data32 & 0x33333333)) << shift;

			layerptr[0] = (layerptr[0] & mask) | data;
			if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else if (blending[4][color]) {
		u64 mask = ~((u64)0xFFFFFFFF << shift);

		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) >= (PPU_TILES_HEIGHT * 8)) break;

			Uint8 ch1 = sprite[v ^ flipy];
			Uint8 ch2 = sprite[(v ^ flipy) | 8];
			u32 data32 = 0;

			if (!flipx) {
				for (h = 0; h < 8; h++) {
					data32 <<= 4;

					Uint8 ch = (ch1 & 1) | ((ch2 & 1) << 1);
					data32 |= blending[ch][color];

					ch1 >>= 1; ch2 >>= 1;
				}
			} else {
				for (h = 0; h < 8; h++) {
					data32 <<= 4;

					Uint8 ch = (ch1 >> 7) | ((ch2 >> 7) << 1);
					data32 |= blending[ch][color];

					ch1 <<= 1; ch2 <<= 1;
				}
			}

			u64 data = ((u64) (data32 & 0x33333333)) << shift;

			layerptr[0] = (layerptr[0] & mask) | data;
			if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else {
		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) >= (PPU_TILES_HEIGHT * 8)) break;

			Uint8 ch1 = sprite[v ^ flipy];
			Uint8 ch2 = sprite[(v ^ flipy) | 8];
			u32 data32 = 0;
			u32 mask32 = 0;

			if (!flipx) {
				for (h = 0; h < 8; h++) {
					data32 <<= 4; mask32 <<= 4;

					if ((ch1 | ch2) & 1) {
						Uint8 ch = (ch1 & 1) | ((ch2 & 1) << 1);
						data32 |= blending[ch][color];
						mask32 |= 0xF;
					}

					ch1 >>= 1; ch2 >>= 1;
				}
			} else {
				for (h = 0; h < 8; h++) {
					data32 <<= 4; mask32 <<= 4;

					if ((ch1 | ch2) & 128) {
						Uint8 ch = (ch1 >> 7) | ((ch2 >> 7) << 1);
						data32 |= blending[ch][color];
						mask32 |= 0xF;
					}

					ch1 <<= 1; ch2 <<= 1;
				}
			}

			u64 data = ((u64) (data32 & 0x33333333)) << shift;
			u64 mask = ~(((u64) (mask32 & 0x33333333)) << shift);

			layerptr[0] = (layerptr[0] & mask) | data;
			if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	}

	tile_dirty[y >> 3] |= dirtyflag;
	tile_dirty[(y + 7) >> 3] |= dirtyflag;
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

typedef struct {
	Uint32 a, b, c, d, e, f, g, h;
} TileBackup;

static inline void
copytile(TileBackup *tptr)
{
	TileBackup *tdstptr = (TileBackup*) (((u32) tptr) & 0xFFFEFFFF);
	*tdstptr = *tptr;
}

ITCM_ARM_CODE
void
copyppu(Ppu *p)
{
	int i, k, ofs;

	for (i = 0; i < 24; i++) {
		if (tile_dirty[i] != 0) {
			while ((k = __builtin_ffs(tile_dirty[i])) > 0) {
				k--;
				ofs = (i << 8) | (k << 3);
				copytile((TileBackup*) (p->bg + ofs));
				copytile((TileBackup*) (p->fg + ofs));
				tile_dirty[i] ^= (1 << k);
			}
		}
	}
}

int
initppu(Ppu *p)
{
	int i;
	u16 *map_ptr;

	videoSetMode(DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE | MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);

	// clear tile data
	p->bg = (Uint32*) BG_TILE_RAM(4);
	p->fg = (Uint32*) BG_TILE_RAM(6);
	for (i = 0; i < 8; i += 2) {
		dmaFillWords(0, BG_TILE_RAM(i), (PPU_TILES_WIDTH * PPU_TILES_HEIGHT) * 32);
	}
	memset(tile_dirty, 0, sizeof(tile_dirty));

	// init bg data
	map_ptr = BG_GFX + (24576 >> 1);
	for (i = 0; i < (PPU_TILES_WIDTH * PPU_TILES_HEIGHT); i++) {
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
