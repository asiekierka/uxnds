/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define FIXED_SIZE 0

typedef struct Layer {
	Uint8 *pixels, changed;
} Layer;

typedef struct UxnScreen {
	Uint32 palette[4], *pixels;
	Uint16 width, height;
	Layer fg, bg;
	Uint8 mono;
} UxnScreen;

extern UxnScreen uxn_screen;

void screen_palette(UxnScreen *p, Uint8 *addr);
void screen_resize(UxnScreen *p, Uint16 width, Uint16 height);
void screen_clear(UxnScreen *p, Layer *layer);
void screen_redraw(UxnScreen *p, Uint32 *pixels);
void screen_mono(UxnScreen *p, Uint32 *pixels);

Uint8 screen_dei(Uint8 *d, Uint8 port);
void screen_deo(Uint8 *ram, Uint8 *d, Uint8 port);
