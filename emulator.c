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

#define HOR 32
#define VER 16
#define PAD 2
#define SZ (HOR * VER * 16)

typedef unsigned char Uint8;

int WIDTH = 8 * HOR + 8 * PAD * 2;
int HEIGHT = 8 * (VER + 2) + 8 * PAD * 2;
int FPS = 30, GUIDES = 1, BIGPIXEL = 0, ZOOM = 2;

Uint32 theme[] = {
	0x000000,
	0xFFFFFF,
	0x72DEC2,
	0x666666,
	0x222222};

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
SDL_Texture *gTexture;
Uint32 *pixels;

Device *devconsole, *devscreen, *devmouse, *devkey, *devsprite, *devctrl;

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
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[y * WIDTH + x] = theme[color];
}

void
drawchr(Uint32 *dst, int x, int y, Uint8 *sprite)
{
	int v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int ch1 = ((sprite[v] >> h) & 0x1);
			int ch2 = (((sprite[v + 8] >> h) & 0x1) << 1);
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
redraw(Uint32 *dst)
{
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
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
	return 1;
}

void
echos(St8 *s, Uint8 len, char *name)
{
	int i;
	printf("\n%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x%c", s->dat[i], s->ptr == i ? '<' : ' ');
	}
	printf("\n\n");
}

void
echom(Memory *m, Uint8 len, char *name)
{
	int i;
	printf("\n%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", m->dat[i]);
	}
	printf("\n\n");
}

void
echof(Uxn *c)
{
	printf("ended @ %d steps | hf: %x sf: %x sf: %x cf: %x\n",
		c->counter,
		getflag(&c->status, FLAG_HALT) != 0,
		getflag(&c->status, FLAG_SHORT) != 0,
		getflag(&c->status, FLAG_SIGN) != 0,
		getflag(&c->status, FLAG_COND) != 0);
}

void
domouse(SDL_Event *event)
{
	int x = event->motion.x / ZOOM;
	int y = event->motion.y / ZOOM;
	devmouse->mem[0] = (x >> 8) & 0xff;
	devmouse->mem[1] = x & 0xff;
	devmouse->mem[2] = (y >> 8) & 0xff;
	devmouse->mem[3] = y & 0xff;
	switch(event->type) {
	case SDL_MOUSEBUTTONUP:
		devmouse->mem[4] = 0;
		break;
	case SDL_MOUSEBUTTONDOWN:
		devmouse->mem[4] = event->button.button == SDL_BUTTON_LEFT;
	}
}

void
dokey(SDL_Event *event)
{
	(void)event;
}

void
doctrl(SDL_Event *event, int z)
{
	Uint8 flag = 0x00;
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
	setflag(&devctrl->mem[0], flag, z);
}

#pragma mark - Devices

Uint8
consoler(Device *d, Memory *m, Uint8 b)
{
	(void)b;
	(void)d;
	(void)m;
	return 0;
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
ppur(Device *d, Memory *m, Uint8 b)
{
	switch(b) {
	case 0: return (WIDTH >> 8) & 0xff;
	case 1: return WIDTH & 0xff;
	case 2: return (HEIGHT >> 8) & 0xff;
	case 3: return HEIGHT & 0xff;
	}
	return d->mem[b];
}

Uint8
ppuw(Device *d, Memory *m, Uint8 b)
{
	d->mem[d->len++] = b;
	if(d->len > 5) {
		putpixel(pixels,
			(d->mem[2] << 8) + d->mem[3],
			(d->mem[0] << 8) + d->mem[1],
			d->mem[4]);
		if(d->mem[5])
			redraw(pixels);
		d->len = 0;
	}
	return 0;
}

Uint8
ppusr(Device *d, Memory *m, Uint8 b)
{
	return 0;
}

Uint8
ppusw(Device *d, Memory *m, Uint8 b)
{
	d->mem[d->len++] = b;
	if(d->len > 6) {
		Uint16 x = (d->mem[2] << 8) + d->mem[3];
		Uint16 y = (d->mem[0] << 8) + d->mem[1];
		Uint8 *chr = &m->dat[(d->mem[4] << 8) + d->mem[5]];
		drawchr(pixels, x, y, chr);
		if(d->mem[6])
			redraw(pixels);
		d->len = 0;
	}
	return 0;
}

Uint8
mouser(Device *d, Memory *m, Uint8 b)
{
	return d->mem[b];
}

Uint8
mousew(Device *d, Memory *m, Uint8 b)
{
	(void)d;
	(void)b;
	return 0;
}

Uint8
keyr(Device *d, Memory *m, Uint8 b)
{
	(void)d;
	(void)b;
	return 0;
}

Uint8
keyw(Device *d, Memory *m, Uint8 b)
{
	(void)d;
	(void)b;
	return 0;
}

Uint8
ctrlr(Device *d, Memory *m, Uint8 b)
{
	return d->mem[b];
}

Uint8
ctrlw(Device *d, Memory *m, Uint8 b)
{
	(void)d;
	(void)b;
	return 0;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	int ticknext = 0;
	evaluxn(u, u->vreset);

	echos(&u->wst, 0x40, "stack");
	echom(&u->ram, 0x40, "ram");
	echof(u);

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
					redraw(pixels);
				break;
			}
		}
		evaluxn(u, u->vframe);
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

	devconsole = portuxn(&u, "console", consoler, consolew);
	devscreen = portuxn(&u, "ppu", ppur, ppuw);
	devmouse = portuxn(&u, "mouse", mouser, mousew);
	devkey = portuxn(&u, "key", keyr, keyw);
	devsprite = portuxn(&u, "ppu-sprite", ppusr, ppusw);
	devctrl = portuxn(&u, "ctrl", ctrlr, ctrlw);

	start(&u);

	echos(&u.wst, 0x40, "stack");
	echom(&u.ram, 0x40, "ram");
	echof(&u);

	quit();
	return 0;
}
