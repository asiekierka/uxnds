#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"
#include "ppu.h"

int initapu(Uxn *u, Uint8 id);

static Ppu screen;

static SDL_Window *gWindow;
static SDL_Renderer *gRenderer;
static SDL_Texture *gTexture;
SDL_AudioDeviceID audio_id;
static Device *devsystem, *devscreen, *devmouse, *devkey, *devctrl;

#pragma mark - Helpers

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

void
setflag(Uint8 *a, char flag, int b)
{
	if(b)
		*a |= flag;
	else
		*a &= (~flag);
}

int
getflag(Uint8 *a, char flag)
{
	return *a & flag;
}

#pragma mark - Core

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

void
redraw(Uint32 *dst, Uxn *u)
{
	Uint16 x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x) {
			Uint16 key = (y * HOR + x) * 16;
			drawchr(&screen, (x + PAD) * 8, (y + PAD) * 8, &screen.bg[key], 0);
			drawchr(&screen, (x + PAD) * 8, (y + PAD) * 8, &screen.fg[key], 1);
		}
	if(screen.debugger)
		drawdebugger(&screen, u->wst.dat, u->wst.ptr);
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
	screen.reqdraw = 0;
}

void
toggledebug(Uxn *u)
{
	screen.debugger = !screen.debugger;
	redraw(screen.output, u);
}

void
togglezoom(Uxn *u)
{
	screen.zoom = screen.zoom == 3 ? 1 : screen.zoom + 1;
	SDL_SetWindowSize(gWindow, WIDTH * screen.zoom, HEIGHT * screen.zoom);
	redraw(screen.output, u);
}

void
quit(void)
{
	free(screen.output);
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
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Uxn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * screen.zoom, HEIGHT * screen.zoom, SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);
	if(!initppu(&screen))
		return error("PPU", "Init failure");
	return 1;
}

void
domouse(Uxn *u, SDL_Event *event)
{
	Uint8 flag = 0x00;
	Uint16 addr = devmouse->addr + 2;
	Uint16 x = clamp(event->motion.x / screen.zoom - PAD * 8, 0, HOR * 8 - 1);
	Uint16 y = clamp(event->motion.y / screen.zoom - PAD * 8, 0, VER * 8 - 1);
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
		if(flag == 0x10 && getflag(&u->ram.dat[addr + 4], 0x01))
			u->ram.dat[addr + 5] = 0x01;
		if(flag == 0x01 && getflag(&u->ram.dat[addr + 4], 0x10))
			u->ram.dat[addr + 5] = 0x10;
		break;
	}
}

void
dotext(Uxn *u, SDL_Event *event)
{
	int i;
	Uint16 addr = devkey->addr + 2;
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
	Uint16 addr = devctrl->addr + 2;
	if(z && event->key.keysym.sym == SDLK_h) {
		if(SDL_GetModState() & KMOD_LCTRL)
			toggledebug(u);
		if(SDL_GetModState() & KMOD_LALT)
			togglezoom(u);
	}
	switch(event->key.keysym.sym) {
	case SDLK_LCTRL: flag = 0x01; break;
	case SDLK_LALT: flag = 0x02; break;
	case SDLK_BACKSPACE:
		flag = 0x04;
		if(z) u->ram.dat[devkey->addr + 2] = 0x08;
		break;
	case SDLK_RETURN:
		flag = 0x08;
		if(z) u->ram.dat[devkey->addr + 2] = 0x0d;
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
console_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	switch(b0) {
	case 0x08: printf("%c", b1); break;
	case 0x09: printf("0x%02x\n", b1); break;
	case 0x0b: printf("0x%04x\n", (m[ptr + 0x0a] << 8) + b1); break;
	case 0x0d: printf("%s\n", &m[(m[ptr + 0x0c] << 8) + b1]); break;
	}
	fflush(stdout);
	(void)m;
	(void)ptr;
	(void)b0;
	return b1;
}

Uint8
screen_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
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
sprite_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	ptr += 8;
	if(b0 == 0x0e) {
		Uint16 v, h;
		Uint16 x = (m[ptr] << 8) + m[ptr + 1];
		Uint16 y = (m[ptr + 2] << 8) + m[ptr + 3];
		Uint8 blend = b1 & 0xf;
		Uint8 *layer = ((b1 >> 4) & 0xf) % 2 ? screen.fg : screen.bg;
		Uint8 *sprite = &m[(m[ptr + 4] << 8) + m[ptr + 5]];
		for(v = 0; v < 8; v++)
			for(h = 0; h < 8; h++) {
				Uint8 ch1 = ((sprite[v] >> (7 - h)) & 0x1);
				if(ch1 == 0 && (blend == 0x05 || blend == 0x0a || blend == 0x0f))
					continue;
				paintpixel(layer, x + h, y + v, ch1 ? blend % 4 : blend / 4);
			}
		screen.reqdraw = 1;
	}
	return b1;
}

Uint8
file_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	char *name = (char *)&m[(m[ptr + 8] << 8) + m[ptr + 8 + 1]];
	Uint16 length = (m[ptr + 8 + 2] << 8) + m[ptr + 8 + 3];
	Uint16 offset = (m[ptr + 0] << 8) + m[ptr + 1];
	if(b0 == 0x0d) {
		Uint16 addr = (m[ptr + 8 + 4] << 8) + b1;
		FILE *f = fopen(name, "r");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fread(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Loaded %d bytes, at %04x from %s\n", length, addr, name);
		}
	} else if(b0 == 0x0f) {
		Uint16 addr = (m[ptr + 8 + 6] << 8) + b1;
		FILE *f = fopen(name, (m[ptr + 2] & 0x1) ? "a" : "w");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fwrite(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Saved %d bytes, at %04x from %s\n", length, addr, name);
		}
	}
	return b1;
}

Uint8
midi_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)u;
	printf("%04x - %02x,%02x\n", ptr, b0, b1);
	return b1;
}

Uint8
datetime_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	time_t seconds = time(NULL);
	struct tm *t = localtime(&seconds);
	t->tm_year += 1900;
	m[ptr + 0] = (t->tm_year & 0xff00) >> 8;
	m[ptr + 1] = t->tm_year & 0xff;
	m[ptr + 2] = t->tm_mon;
	m[ptr + 3] = t->tm_mday;
	m[ptr + 4] = t->tm_hour;
	m[ptr + 5] = t->tm_min;
	m[ptr + 6] = t->tm_sec;
	m[ptr + 7] = t->tm_wday;
	m[ptr + 8] = (t->tm_yday & 0xff00) >> 8;
	m[ptr + 9] = t->tm_yday & 0xff;
	m[ptr + 10] = t->tm_isdst;
	(void)b0;
	return b1;
}

Uint8
system_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	m[PAGE_DEVICE + b0] = b1;
	loadtheme(&screen, &m[PAGE_DEVICE + 0x0008]);
	(void)ptr;
	return b1;
}

Uint8
ppnil(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)u;
	(void)ptr;
	(void)b0;
	return b1;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	inituxn(u, 0x0200);
	redraw(screen.output, u);
	while(1) {
		SDL_Event event;
		double elapsed, start = SDL_GetPerformanceCounter();
		SDL_LockAudioDevice(audio_id);
		while(SDL_PollEvent(&event) != 0) {
			switch(event.type) {
			case SDL_QUIT:
				SDL_UnlockAudioDevice(audio_id);
				quit();
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				domouse(u, &event);
				evaluxn(u, devmouse->vector);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				doctrl(u, &event, event.type == SDL_KEYDOWN);
				evaluxn(u, devctrl->vector);
				break;
			case SDL_TEXTINPUT:
				dotext(u, &event);
				evaluxn(u, devkey->vector);
				break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(screen.output, u);
				break;
			}
		}
		evaluxn(u, devscreen->vector);
		SDL_UnlockAudioDevice(audio_id);
		if(screen.reqdraw)
			redraw(screen.output, u);
		elapsed = (SDL_GetPerformanceCounter() - start) / (double)SDL_GetPerformanceFrequency() * 1000.0f;
		SDL_Delay(clamp(16.666f - elapsed, 0, 1000));
	}
	return 1;
}

int
main(int argc, char **argv)
{
	Uxn u;
	screen.zoom = 2;

	if(argc < 2)
		return error("Input", "Missing");
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, argv[1]))
		return error("Load", "Failed");
	if(!init())
		return error("Init", "Failed");

	devsystem = portuxn(&u, 0x00, "system", system_poke);
	portuxn(&u, 0x01, "console", console_poke);
	devscreen = portuxn(&u, 0x02, "screen", screen_poke);
	portuxn(&u, 0x03, "sprite", sprite_poke);
	devctrl = portuxn(&u, 0x04, "controller", ppnil);
	devkey = portuxn(&u, 0x05, "key", ppnil);
	devmouse = portuxn(&u, 0x06, "mouse", ppnil);
	portuxn(&u, 0x07, "file", file_poke);
	if(!initapu(&u, 0x08))
		return 1;
	portuxn(&u, 0x09, "midi", ppnil);
	portuxn(&u, 0x0a, "datetime", datetime_poke);
	portuxn(&u, 0x0b, "---", ppnil);
	portuxn(&u, 0x0c, "---", ppnil);
	portuxn(&u, 0x0d, "---", ppnil);
	portuxn(&u, 0x0e, "---", ppnil);
	portuxn(&u, 0x0f, "---", ppnil);

	/* Write screen size to dev/screen */
	u.ram.dat[devscreen->addr + 2] = (HOR * 8 >> 8) & 0xff;
	u.ram.dat[devscreen->addr + 3] = HOR * 8 & 0xff;
	u.ram.dat[devscreen->addr + 4] = (VER * 8 >> 8) & 0xff;
	u.ram.dat[devscreen->addr + 5] = VER * 8 & 0xff;

	start(&u);
	quit();
	return 0;
}
