#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned int Uint32;

#define HOR 48
#define VER 32
#define PAD 2
#define RES (HOR * VER * 16)

#define WIDTH (8 * HOR + 8 * PAD * 2)
#define HEIGHT (8 * VER + 8 * PAD * 2)

typedef struct Ppu {
	Uint8 reqdraw, zoom, debugger, bg[RES], fg[RES];
	Uint16 x1, y1, x2, y2;
	Uint32 *output, colors[4];
} Ppu;

void
clear(Ppu *p);
void
drawpixel(Ppu *p, Uint16 x, Uint16 y, Uint8 color);
void
drawchr(Ppu *p, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 alpha);
void
drawicn(Ppu *p, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 fg, Uint8 bg);

void paintpixel(Uint8 *dst, Uint16 x, Uint16 y, Uint8 color);
void loadtheme(Ppu *p, Uint8 *addr);