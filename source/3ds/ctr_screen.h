/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <tex3ds.h>

typedef struct Layer {
	int y1, y2;
	Uint32 palette[4];
	Uint32 *gpuPixels;
	C2D_Image gpuImage;
	C3D_Tex gpuTexture;
	Uint8 *pixels, changed;
} Layer;

typedef struct UxnCtrScreen {
	int width, height, pitch;
	Layer fg, bg;
	Tex3DS_SubTexture gpuSubTexture;
} UxnCtrScreen;

extern UxnCtrScreen uxn_ctr_screen;

void ctr_screen_palette(UxnCtrScreen *p, Uint8 *addr);
void ctr_screen_free(UxnCtrScreen *p);
void ctr_screen_init(UxnCtrScreen *p, int width, int height);
void ctr_screen_redraw(UxnCtrScreen *p);

Uint8 ctr_screen_dei(Uxn *u, Uint8 addr);
void ctr_screen_deo(Uint8 *ram, Uint8 *d, Uint8 port);
