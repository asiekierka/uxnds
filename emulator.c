#include <SDL2/SDL.h>
#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"

#define HOR 64 / 2
#define VER 48 / 2
#define PAD 2
#define RES (HOR * VER * 16)

typedef struct {
	Uint8 reqdraw;
	Uint8 bg[RES], fg[RES];
} Screen;

int WIDTH = 8 * HOR + 8 * PAD * 2;
int HEIGHT = 8 * VER + 8 * PAD * 2;
int FPS = 30, GUIDES = 1, ZOOM = 2;

Uint32 theme[] = {
	0x000000,
	0xFFFFFF,
	0x72DEC2,
	0x666666,
	0x222222};

Uint8 icons[][8] = {
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

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
SDL_Texture *gTexture;
Uint32 *pixels;

Screen screen;
Device *devconsole, *devscreen, *devmouse, *devkey, *devsprite, *devcontroller;

#pragma mark - Helpers

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

#pragma mark - Paint

Uint16
rowchr(Uint16 x, Uint16 y)
{
	return (y % 8) + ((x / 8 + y / 8 * HOR) * 16);
}

void
paintpixel(Uint8 *dst, Uint16 x, Uint16 y, Uint8 color)
{
	Uint16 row = rowchr(x, y), col = x % 8;
	if(x >= HOR * 8 || y >= VER * 8 || row > RES - 8)
		return;
	if(color == 0 || color == 2)
		dst[row] &= ~(1UL << (7 - col));
	else
		dst[row] |= 1UL << (7 - col);
	if(color == 0 || color == 1)
		dst[row + 8] &= ~(1UL << (7 - col));
	else
		dst[row + 8] |= 1UL << (7 - col);
}

void
paintchr(Uint8 *dst, Uint16 x, Uint16 y, Uint8 *sprite)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
			Uint8 ch2 = (((sprite[v + 8] >> (7 - h)) & 0x1) << 1);
			paintpixel(dst, x + h, y + v, ch1 + ch2);
		}
}

void
painticn(Uint8 *dst, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 blend)
{
	Uint16 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
			if(ch1 == 0 && (blend == 0x05 || blend == 0x0a || blend == 0x0f))
				continue;
			paintpixel(dst, x + h, y + v, ch1 ? blend % 4 : blend / 4);
		}
}

#pragma mark - Helpers

void
clear(Uint32 *dst)
{
	int v, h;
	for(v = 0; v < HEIGHT; v++)
		for(h = 0; h < WIDTH; h++)
			dst[v * WIDTH + h] = theme[0];
}

void
putpixel(Uint32 *dst, int x, int y, int color)
{
	if(y == PAD * 8 || y == HEIGHT - PAD * 8 - 1) {
		if(x == PAD * 8) return;
		if(x == WIDTH - PAD * 8 - 1) return;
	}
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[y * WIDTH + x] = theme[color];
}

void
drawchr(Uint32 *dst, int x, int y, Uint8 *sprite, Uint8 alpha)
{
	int v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> h) & 0x1);
			Uint8 ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
			if(!alpha || (alpha && ch1 + ch2 != 0))
				putpixel(dst, x + 7 - h, y + v, ch1 + ch2);
		}
}

void
drawicn(Uint32 *dst, int x, int y, Uint8 *sprite, int fg, int bg)
{
	int v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int ch1 = (sprite[v] >> (7 - h)) & 0x1;
			putpixel(dst, x + h, y + v, ch1 ? fg : bg);
		}
}

#pragma mark - Core

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

void
loadtheme(Uint8 *addr)
{
	Uint8 r, g, b;
	r = *(addr + 0) >> 4 & 0xf, g = *(addr + 2) >> 4 & 0xf, b = *(addr + 4) >> 4 & 0xf;
	theme[0] = ((r + (r << 4)) << 16) + ((g + (g << 4)) << 8) + (b + (b << 4));
	r = *(addr + 0) & 0xf, g = *(addr + 2) & 0xf, b = *(addr + 4) & 0xf;
	theme[1] = ((r + (r << 4)) << 16) + ((g + (g << 4)) << 8) + (b + (b << 4));
	r = *(addr + 1) >> 4 & 0xf, g = *(addr + 3) >> 4 & 0xf, b = *(addr + 5) >> 4 & 0xf;
	theme[2] = ((r + (r << 4)) << 16) + ((g + (g << 4)) << 8) + (b + (b << 4));
	r = *(addr + 1) & 0xf, g = *(addr + 3) & 0xf, b = *(addr + 5) & 0xf;
	theme[3] = ((r + (r << 4)) << 16) + ((g + (g << 4)) << 8) + (b + (b << 4));
}

void
drawdebugger(Uint32 *dst, Uxn *u)
{
	Uint8 i;
	for(i = 0; i < 0x10; ++i) { /* memory */
		Uint8 x = (i % 8) * 3 + 3, y = i / 8 + 3, b = u->ram.dat[i];
		drawicn(dst, x * 8, y * 8, icons[(b >> 4) & 0xf], 1, 0);
		drawicn(dst, x * 8 + 8, y * 8, icons[b & 0xf], 1, 0);
		y = VER - i / 8, b = u->wst.dat[i];
		drawicn(dst, x * 8, y * 8, icons[(b >> 4) & 0xf], 1 + (u->wst.ptr == i), 0);
		drawicn(dst, x * 8 + 8, y * 8, icons[b & 0xf], 1 + (u->wst.ptr == i), 0);
	}
}

void
redraw(Uint32 *dst, Uxn *u)
{
	int x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x) {
			int key = (y * HOR + x) * 16;
			drawchr(dst, (x + PAD) * 8, (y + PAD) * 8, &screen.bg[key], 0);
			drawchr(dst, (x + PAD) * 8, (y + PAD) * 8, &screen.fg[key], 1);
		}
	if(GUIDES)
		drawdebugger(dst, u);
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
	screen.reqdraw = 0;
}

void
quit(void)
{
	free(pixels);
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
	exit(0);
}

int
init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Uxn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * ZOOM, HEIGHT * ZOOM, SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	if(!(pixels = (Uint32 *)malloc(WIDTH * HEIGHT * sizeof(Uint32))))
		return error("Pixels", "Failed to allocate memory");
	clear(pixels);
	SDL_ShowCursor(SDL_DISABLE);
	return 1;
}

void
domouse(SDL_Event *event)
{
	int x = clamp((event->motion.x - PAD * 8 * ZOOM) / ZOOM, 0, WIDTH - 1);
	int y = clamp((event->motion.y - PAD * 8 * ZOOM) / ZOOM, 0, HEIGHT - 1);
	devmouse->mem[0] = (x >> 8) & 0xff;
	devmouse->mem[1] = x & 0xff;
	devmouse->mem[2] = (y >> 8) & 0xff;
	devmouse->mem[3] = y & 0xff;
	devmouse->mem[4] = event->button.button == SDL_BUTTON_LEFT;
	devmouse->mem[5] = 0x00;
	switch(event->type) {
	case SDL_MOUSEBUTTONUP:
		devmouse->mem[4] = 0;
		devmouse->mem[5] = 0x10;
		break;
	case SDL_MOUSEBUTTONDOWN:
		devmouse->mem[4] = event->button.button == SDL_BUTTON_LEFT;
		devmouse->mem[5] = 0x01;
		break;
	}
}

void
doctrl(SDL_Event *event, int z)
{
	Uint8 flag = 0x00;
	if(z && event->key.keysym.sym == SDLK_h)
		GUIDES = !GUIDES;
	if(SDL_GetModState() & KMOD_LCTRL || SDL_GetModState() & KMOD_RCTRL)
		flag = 0x01;
	if(SDL_GetModState() & KMOD_LALT || SDL_GetModState() & KMOD_RALT)
		flag = 0x02;
	switch(event->key.keysym.sym) {
	case SDLK_ESCAPE: flag = 0x04; break;
	case SDLK_RETURN: flag = 0x08; break;
	case SDLK_UP: flag = 0x10; break;
	case SDLK_DOWN: flag = 0x20; break;
	case SDLK_LEFT: flag = 0x40; break;
	case SDLK_RIGHT: flag = 0x80; break;
	}
	setflag(&devcontroller->mem[0], flag, z);
}

#pragma mark - Devices

Uint8
defaultrw(Device *d, Memory *m, Uint8 b)
{
	(void)m;
	return d->mem[b];
}

Uint8
consolew(Device *d, Memory *m, Uint8 b)
{
	if(b)
		printf("%c", b);
	fflush(stdout);
	(void)d;
	(void)m;
	return 0;
}

Uint8
screenr(Device *d, Memory *m, Uint8 b)
{
	loadtheme(m->dat + 0xfff0);
	switch(b) {
	case 0: return (HOR * 8 >> 8) & 0xff;
	case 1: return HOR * 8 & 0xff;
	case 2: return (VER * 8 >> 8) & 0xff;
	case 3: return VER * 8 & 0xff;
	}
	(void)m;
	return d->mem[b];
}

Uint8
screenw(Device *d, Memory *m, Uint8 b)
{
	d->mem[d->ptr++] = b;
	if(d->ptr > 4) {
		Uint16 x = (d->mem[2] << 8) + d->mem[3];
		Uint16 y = (d->mem[0] << 8) + d->mem[1];
		Uint8 clr = d->mem[4] & 0xf;
		Uint8 layer = d->mem[4] >> 4 & 0xf;
		paintpixel(layer ? screen.fg : screen.bg, x, y, clr);
		screen.reqdraw = 1;
		d->ptr = 0;
	}
	(void)m;
	return 0;
}

Uint8
spritew(Device *d, Memory *m, Uint8 b)
{
	d->mem[d->ptr++] = b;
	if(d->ptr == 7) {
		Uint16 x = (d->mem[2] << 8) + d->mem[3];
		Uint16 y = (d->mem[0] << 8) + d->mem[1];
		Uint16 a = (d->mem[4] << 8) + d->mem[5];
		Uint8 source = d->mem[6] >> 4 & 0xf;
		Uint8 *layer = source % 2 ? screen.fg : screen.bg;
		if(source / 2)
			paintchr(layer, x, y, &m->dat[a]);
		else
			painticn(layer, x, y, &m->dat[a], d->mem[6] & 0xf);
		screen.reqdraw = 1;
		d->ptr = 0;
	}
	return 0;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	int ticknext = 0;
	evaluxn(u, u->vreset);
	loadtheme(u->ram.dat + 0xfff0);
	if(screen.reqdraw)
		redraw(pixels, u);
	while(1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if(tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);
		while(SDL_PollEvent(&event) != 0) {
			switch(event.type) {
			case SDL_QUIT: quit(); break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION: domouse(&event); break;
			case SDL_KEYDOWN: doctrl(&event, 1); break;
			case SDL_KEYUP: doctrl(&event, 0); break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(pixels, u);
				break;
			}
		}
		evaluxn(u, u->vframe);
		if(screen.reqdraw)
			redraw(pixels, u);
	}
}

int
main(int argc, char **argv)
{
	Uxn u;

	if(argc < 2)
		return error("Input", "Missing");
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, argv[1]))
		return error("Load", "Failed");
	if(!init())
		return error("Init", "Failed");

	devconsole = portuxn(&u, "console", defaultrw, consolew);
	devscreen = portuxn(&u, "screen", screenr, screenw);
	devsprite = portuxn(&u, "sprite", screenr, spritew);
	devcontroller = portuxn(&u, "controller", defaultrw, defaultrw);
	devkey = portuxn(&u, "key", defaultrw, consolew);
	devmouse = portuxn(&u, "mouse", defaultrw, defaultrw);

	start(&u);
	quit();
	return 0;
}
