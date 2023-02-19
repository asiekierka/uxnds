#include <stdio.h>
#include <stdlib.h>

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

#define PPU_TILES_WIDTH 32
#define PPU_TILES_HEIGHT 24
#define PPU_PIXELS_WIDTH (PPU_TILES_WIDTH * 8)
#define PPU_PIXELS_HEIGHT (PPU_TILES_HEIGHT * 8)

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;

typedef struct NdsPpu {
	Uint32 *bg, *fg;
} NdsPpu;

int nds_initppu(NdsPpu *p);
void nds_putcolors(NdsPpu *p, Uint8 *addr);
void nds_ppu_pixel(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 color);
void nds_ppu_2bpp(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy);
void nds_ppu_1bpp(NdsPpu *p, Uint32 *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy);
void nds_copyppu(NdsPpu *p);
