#include <stdio.h>
#include <stdlib.h>

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;

typedef struct Layer {
	Uint32 *pixels, colors[4];
} Layer;

typedef struct Ppu {
	Uint16 hor, ver, width, height;
	Layer fg, bg;
} Ppu;

int initppu(Ppu *p, Uint8 hor, Uint8 ver);
void putcolors(Ppu *p, Uint8 *addr);
void putpixel(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 color);
void puticn(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy);
void putchr(Ppu *p, Layer *layer, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 color, Uint8 flipx, Uint8 flipy);
void drawdebugger(Ppu *p, Uint8 *stack, Uint8 ptr);
