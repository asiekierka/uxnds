#include <nds.h>
#include "uxn.h"
#include "nds_ppu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick
Copyright (c) 2021, 2022, 2023 Adrian "asie" Siekierka
Copyright (c) 2021 Bad Diode

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
        {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0}};

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

DTCM_DATA
static Uint32 lut_expand_8_32_f[256] = {
#include "lut_expand_8_32_f.inc"
};

DTCM_DATA
static Uint32 lut_expand_8_32_f_flipx[256] = {
#include "lut_expand_8_32_f_flipx.inc"
};

/* DTCM_DATA
static Uint32 lut_mask_8_32_count[8] = {
	0x0000000F,
	0x000000FF,
	0x00000FFF,
	0x0000FFFF,
	0x000FFFFF,
	0x00FFFFFF,
	0x0FFFFFFF,
	0xFFFFFFFF
}; */

DTCM_BSS
static Uint16 pal_colors_cache[4];
DTCM_BSS
static bool pal_colors_cache_changed = false;

void
nds_putcolors(NdsPpu *p, Uint8 *addr)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Uint8
			r = (*(addr + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f,
			g = (*(addr + 2 + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f,
			b = (*(addr + 4 + (i >> 1)) >> (!(i & 1) << 2)) & 0x0f;
		pal_colors_cache[i] = RGB15(
			(r << 1) | (r >> 3),
			(g << 1) | (g >> 3),
			(b << 1) | (b >> 3)
		);
	}
	pal_colors_cache_changed = true;
}

ITCM_ARM_CODE
void
nds_ppu_pixel(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= PPU_TILES_WIDTH * 8 || y >= PPU_TILES_HEIGHT * 8)
		return;
	Uint32 pos = ((y & 7) + ( ((x >> 3) + (y >> 3) * PPU_TILES_WIDTH) * 8) );
	Uint32 shift = (x & 7) << 2;
	layer[pos] = (layer[pos] & (~(0xF << shift))) | (color << shift);
	tile_dirty[y >> 3] |= 1 << (x >> 3);
}

// Doesn't calculate tile_dirty by itself.
// Modified from https://git.badd10de.dev/uxngba/tree/src/ppu.c
ITCM_ARM_CODE
static inline void
__nds_ppu_hline(Uint32 *layer, Uint16 x0, Uint16 x1, Uint32 y, Uint32 row) {
	Uint32 y_pos = (y & 7) + ((y >> 3) * PPU_TILES_WIDTH * 8);
	// Find row positions for the given x/y coordinates.
	size_t tile_x0 = x0 >> 3;
	size_t tile_x1 = x1 >> 3;
	size_t start_col = x0 & 7;
	size_t end_col = x1 & 7;

	// Horizontal line. There are 3 cases:
	//     1. Lines fit on a single tile.
	//     2. Lines go through 2 tiles, both require partial row updates.
	//     3. Lines go through 3 or more tiles, first and last tiles use
	//        partial row updates, rows in the middle can write the entire
	//        row.
	size_t dtx = tile_x1 - tile_x0;
	Uint32 *dst = &layer[(x0 & (~7)) + y_pos];

	size_t shift_left = start_col * 4;
	size_t shift_right = (7 - end_col) * 4;

	if (dtx < 1) {
		u32 mask = (0xFFFFFFFF >> shift_right) & (0xFFFFFFFF << shift_left);
		*dst = (*dst & ~mask) | (row & mask);
	} else {
		u32 mask = 0xFFFFFFFF;
		*dst = (*dst & ~(mask << shift_left)) | (row << shift_left);
		dst += 8;
		for (size_t i = 1; i < dtx; i++) {
			*dst = row;
			dst += 8;
		}
		*dst = (*dst & ~(mask >> shift_right)) | (row >> shift_right);
	}
}

ITCM_ARM_CODE
void
nds_ppu_fill(NdsPpu *p, Uint32 *layer, Uint16 x1, Uint16 y1, Uint16 x2, Uint16 y2, Uint8 color)
{
	if (x1 > PPU_TILES_WIDTH * 8)  x1 = PPU_TILES_WIDTH * 8;
	if (x2 > PPU_TILES_WIDTH * 8)  x2 = PPU_TILES_WIDTH * 8;
	if (y1 > PPU_TILES_HEIGHT * 8) y1 = PPU_TILES_HEIGHT * 8;
	if (y2 > PPU_TILES_HEIGHT * 8) y2 = PPU_TILES_HEIGHT * 8;

	Uint32 color_full = 0x11111111 * color;

	for (Uint16 y = y1; y < y2; y++) {
#if 1
		__nds_ppu_hline(layer, x1, x2, y, color_full);
#else
		Uint32 y_pos = (y & 7) + ((y >> 3) * PPU_TILES_WIDTH * 8);

		// TODO: write a more optimized implementation
		for (Uint16 x = x1; x < x2; x++) {
			if (!(x & 0x7) && ((x2 - x) >= 8)) {
				layer[y_pos + x] = color_full;
				x += 7;
			} else {
				Uint32 pos = y_pos + (x & (~7));
				Uint32 shift = (x & 7) << 2;
				layer[pos] = (layer[pos] & (~(0xF << shift))) | (color << shift);
			}
		}
#endif
	}

	// calculate tile_dirty
	{
		Uint32 dirty_line = 0;
		for (Uint16 x = x1 >> 3; x <= (x2 - 1) >> 3; x++) {
			dirty_line |= (1 << x);
		}
		for (Uint16 y = y1 >> 3; y <= (y2 - 1) >> 3; y++) {
			tile_dirty[y] |= dirty_line;
		}
	}
}

ITCM_ARM_CODE
void
nds_ppu_1bpp(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
{
	Uint8 sprline;
	Uint8 xrightedge = x < ((PPU_TILES_WIDTH - 1) * 8);
	Uint16 v;
	Uint32 dirtyflag = (1 << (x >> 3)) | (1 << ((x + 7) >> 3));

	Uint32 layerpos = ((y & 7) + (((x >> 3) + (y >> 3) * PPU_TILES_WIDTH) * 8));
	Uint32 *layerptr = &layer[layerpos];
	Uint32 shift = (x & 7) << 2;
	Uint32 *lut_expand = flipx ? lut_expand_8_32_f : lut_expand_8_32_f_flipx;
	Uint32 color_fg = (color & 3) * 0x11111111;
	Uint32 color_bg = (color >> 2) * 0x11111111;

	if (flipy) flipy = 7;

	if(x >= PPU_TILES_WIDTH * 8 || y >= PPU_TILES_HEIGHT * 8)
		return;

	if (blending[4][color]) {
		u64 mask = ~((u64)0xFFFFFFFF << shift);

		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) < (PPU_TILES_HEIGHT * 8)) {
				sprline = sprite[v ^ flipy];
				u64 data = (u64)(lut_expand[sprline] & color_fg) << shift;
				data |= (u64)(lut_expand[sprline ^ 0xFF] & color_bg) << shift;

				layerptr[0] = (layerptr[0] & mask) | data;
				if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);
			} else if (!flipy) break;

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else {
		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) < (PPU_TILES_HEIGHT * 8)) {
				sprline = sprite[v ^ flipy];
				u64 mask = ~((u64)(lut_expand[sprline]) << shift);
				u64 data = (u64)(lut_expand[sprline] & color_fg) << shift;

				layerptr[0] = (layerptr[0] & mask) | data;
				if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);
			} else if (!flipy) break;

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
nds_ppu_2bpp(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy)
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
			if ((y + v) < (PPU_TILES_HEIGHT * 8)) {
				sprline1 = sprite[v ^ flipy];
				sprline2 = sprite[(v ^ flipy) | 8];

				u32 data32 = (lut_expand[sprline1]) | ((lut_expand[sprline2]) << 1);
				u64 data = ((u64) (data32)) << shift;

				layerptr[0] = (layerptr[0] & mask) | data;
				if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);
			} else if (!flipy) break;

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else if (blending[4][color]) {
		u64 mask = ~((u64)0xFFFFFFFF << shift);

		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) < (PPU_TILES_HEIGHT * 8)) {
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

				u64 data = ((u64) (data32)) << shift;

				layerptr[0] = (layerptr[0] & mask) | data;
				if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);
			} else if (!flipy) break;

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	} else {
		for (v = 0; v < 8; v++, layerptr++) {
			if ((y + v) < (PPU_TILES_HEIGHT * 8)) {
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
							mask32 |= 0x3;
						}

						ch1 >>= 1; ch2 >>= 1;
					}
				} else {
					for (h = 0; h < 8; h++) {
						data32 <<= 4; mask32 <<= 4;

						if ((ch1 | ch2) & 128) {
							Uint8 ch = (ch1 >> 7) | ((ch2 >> 7) << 1);
							data32 |= blending[ch][color];
							mask32 |= 0x3;
						}

						ch1 <<= 1; ch2 <<= 1;
					}
				}

				u64 data = ((u64) (data32)) << shift;
				u64 mask = ~(((u64) (mask32)) << shift);

				layerptr[0] = (layerptr[0] & mask) | data;
				if (xrightedge) layerptr[8] = (layerptr[8] & (mask >> 32)) | (data >> 32);
			} else if (!flipy) break;

			if (((y + v) & 7) == 7) layerptr += (PPU_TILES_WIDTH - 1) * 8;
		}
	}

	tile_dirty[y >> 3] |= dirtyflag;
	tile_dirty[(y + 7) >> 3] |= dirtyflag;
}

/* output */

/* void
drawdebugger(NdsPpu *p, Uint8 *stack, Uint8 ptr)
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
nds_copyppu(NdsPpu *p)
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

	if (pal_colors_cache_changed) {
		BG_PALETTE[0] = pal_colors_cache[0];
		BG_PALETTE[1] = pal_colors_cache[1];
		BG_PALETTE[2] = pal_colors_cache[2];
		BG_PALETTE[3] = pal_colors_cache[3];
		pal_colors_cache_changed = false;
	}
}

int
nds_initppu(NdsPpu *p)
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
