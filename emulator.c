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

#define HOR 48
#define VER 32
#define PAD 2
#define RES (HOR * VER * 16)

typedef struct {
	Uint16 x1, y1, x2, y2;
} Rect2d;

typedef struct {
	Uint8 reqdraw;
	Uint8 bg[RES], fg[RES];
	Rect2d bounds;
} Screen;

int WIDTH = 8 * HOR + 8 * PAD * 2;
int HEIGHT = 8 * VER + 8 * PAD * 2;
int FPS = 30, GUIDES = 0, ZOOM = 2;

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
Device *devconsole, *devscreen, *devmouse, *devkey, *devsprite, *devctrl, *devsystem;

#pragma mark - Helpers

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

#pragma mark - Paint

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
drawpixel(Uint32 *dst, Uint16 x, Uint16 y, Uint8 color)
{
	if(x >= screen.bounds.x1 && x <= screen.bounds.x2 && y >= screen.bounds.x1 && y <= screen.bounds.y2)
		dst[y * WIDTH + x] = theme[color];
}

void
drawchr(Uint32 *dst, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 alpha)
{
	Uint8 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = ((sprite[v] >> h) & 0x1);
			Uint8 ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
			if(!alpha || (alpha && ch1 + ch2 != 0))
				drawpixel(dst, x + 7 - h, y + v, ch1 + ch2);
		}
}

void
drawicn(Uint32 *dst, Uint16 x, Uint16 y, Uint8 *sprite, Uint8 fg, Uint8 bg)
{
	Uint8 v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			Uint8 ch1 = (sprite[v] >> (7 - h)) & 0x1;
			drawpixel(dst, x + h, y + v, ch1 ? fg : bg);
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
	screen.reqdraw = 1;
}

void
drawdebugger(Uint32 *dst, Uxn *u)
{
	Uint8 i, x, y, b;
	for(i = 0; i < 0x10; ++i) { /* memory */
		x = ((i % 8) * 3 + 3) * 8, y = screen.bounds.x1 + 8 + i / 8 * 8, b = u->wst.dat[i];
		drawicn(dst, x, y, icons[(b >> 4) & 0xf], 1 + (u->wst.ptr == i), 0);
		drawicn(dst, x + 8, y, icons[b & 0xf], 1 + (u->wst.ptr == i), 0);
	}
}

void
redraw(Uint32 *dst, Uxn *u)
{
	Uint16 x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x) {
			Uint16 key = (y * HOR + x) * 16;
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
	screen.bounds.x1 = PAD * 8;
	screen.bounds.x2 = WIDTH - PAD * 8 - 1;
	screen.bounds.y1 = PAD * 8;
	screen.bounds.y2 = HEIGHT - PAD * 8 - 1;
	return 1;
}

void
domouse(Uxn *u, SDL_Event *event)
{
	Uint8 flag = 0x00;
	Uint16 addr = devmouse->addr;
	Uint16 x = clamp(event->motion.x / ZOOM - PAD * 8, 0, HOR * 8 - 1);
	Uint16 y = clamp(event->motion.y / ZOOM - PAD * 8, 0, VER * 8 - 1);
	u->ram.dat[addr + 0] = (x >> 8) & 0xff;
	u->ram.dat[addr + 1] = x & 0xff;
	u->ram.dat[addr + 2] = (y >> 8) & 0xff;
	u->ram.dat[addr + 3] = y & 0xff;
	u->ram.dat[addr + 5] = 0x00;
	switch(event->button.button) {
	case SDL_BUTTON_LEFT: flag = 0x01; break;
	case SDL_BUTTON_RIGHT: flag = 0x10; break;
	}
	switch(event->type) {
	case SDL_MOUSEBUTTONUP:
		setflag(&u->ram.dat[addr + 4], flag, 0);
		break;
	case SDL_MOUSEBUTTONDOWN:
		setflag(&u->ram.dat[addr + 4], flag, 1);
		if(flag == 0x01 && getflag(&u->ram.dat[addr + 4], 0x10))
			u->ram.dat[addr + 5] = 0x01;
		if(flag == 0x10 && getflag(&u->ram.dat[addr + 4], 0x01))
			u->ram.dat[addr + 5] = 0x10;
		break;
	}
}

void
dotext(Uxn *u, SDL_Event *event)
{
	int i;
	Uint16 addr = devkey->addr;
	if(SDL_GetModState() & KMOD_LCTRL || SDL_GetModState() & KMOD_RCTRL)
		return;
	for(i = 0; i < SDL_TEXTINPUTEVENT_TEXT_SIZE; ++i) {
		char c = event->text.text[i];
		if(c < ' ' || c > '~')
			break;
		u->ram.dat[addr] = c;
	}
}

void
doctrl(Uxn *u, SDL_Event *event, int z)
{
	Uint8 flag = 0x00;
	Uint16 addr = devctrl->addr;
	if(z && event->key.keysym.sym == SDLK_h && SDL_GetModState() & KMOD_LCTRL)
		GUIDES = !GUIDES;
	switch(event->key.keysym.sym) {
	case SDLK_LCTRL: flag = 0x01; break;
	case SDLK_LALT: flag = 0x02; break;
	case SDLK_BACKSPACE:
		flag = 0x04;
		if(z) u->ram.dat[devkey->addr] = 0x08;
		break;
	case SDLK_RETURN:
		flag = 0x08;
		if(z) u->ram.dat[devkey->addr] = 0x0d;
		break;
	case SDLK_UP: flag = 0x10; break;
	case SDLK_DOWN: flag = 0x20; break;
	case SDLK_LEFT: flag = 0x40; break;
	case SDLK_RIGHT: flag = 0x80; break;
	}
	setflag(&u->ram.dat[addr], flag, z);
}

#pragma mark - Devices

Uint8
console_poke(Uint8 *m, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	switch(b0) {
	case 0x08: printf("%c", b1); break;
	case 0x09: printf("%02x", b1); break;
	case 0x0b: printf("%04x", (m[ptr + 0x0a] << 8) + b1); break;
	}
	fflush(stdout);
	(void)m;
	(void)ptr;
	(void)b0;
	return b1;
}

Uint8
screen_poke(Uint8 *m, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	ptr += 8;
	if(b0 == 0x0c) {
		Uint16 x = (m[ptr] << 8) + m[ptr + 1];
		Uint16 y = (m[ptr + 2] << 8) + m[ptr + 3];
		paintpixel(b1 >> 4 & 0xf ? screen.fg : screen.bg, x, y, b1 & 0xf);
		screen.reqdraw = 1;
	}
	return b1;
}

Uint8
sprite_poke(Uint8 *m, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	ptr += 8;
	if(b0 == 0x0e) {
		Uint16 x = (m[ptr] << 8) + m[ptr + 1];
		Uint16 y = (m[ptr + 2] << 8) + m[ptr + 3];
		Uint16 a = (m[ptr + 4] << 8) + m[ptr + 5];
		Uint8 source = (b1 >> 4) & 0xf;
		Uint8 *layer = source % 2 ? screen.fg : screen.bg;
		if(source / 2)
			paintchr(layer, x, y, &m[a]);
		else
			painticn(layer, x, y, &m[a], b1 & 0xf);
		screen.reqdraw = 1;
	}
	return b1;
}

Uint8
system_poke(Uint8 *m, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	loadtheme(&m[0xfff8]);
	(void)ptr;
	(void)b0;
	return b1;
}

Uint8
ppnil(Uint8 *m, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)m;
	(void)ptr;
	(void)b0;
	return b1;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	int ticknext = 0;
	evaluxn(u, u->vreset);
	loadtheme(u->ram.dat + 0xfff8);
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
			case SDL_MOUSEMOTION: domouse(u, &event); break;
			case SDL_TEXTINPUT: dotext(u, &event); break;
			case SDL_KEYDOWN: doctrl(u, &event, 1); break;
			case SDL_KEYUP: doctrl(u, &event, 0); break;
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

	devconsole = portuxn(&u, "console", ppnil, console_poke);
	devscreen = portuxn(&u, "screen", ppnil, screen_poke);
	devsprite = portuxn(&u, "sprite", ppnil, sprite_poke);
	devctrl = portuxn(&u, "controller", ppnil, ppnil);
	devkey = portuxn(&u, "key", ppnil, ppnil);
	devmouse = portuxn(&u, "mouse", ppnil, ppnil);

	u.devices = 15; /* pad to last device */
	devsystem = portuxn(&u, "system", ppnil, system_poke);

	/* Write screen size to dev/screen */
	u.ram.dat[devscreen->addr + 0] = (HOR * 8 >> 8) & 0xff;
	u.ram.dat[devscreen->addr + 1] = HOR * 8 & 0xff;
	u.ram.dat[devscreen->addr + 2] = (VER * 8 >> 8) & 0xff;
	u.ram.dat[devscreen->addr + 3] = VER * 8 & 0xff;

	start(&u);
	quit();
	return 0;
}
