#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "uxn.h"
#include "devices/ppu.h"
#include "devices/apu.h"
#include "devices/mpu.h"

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static SDL_AudioDeviceID audio_id;
static SDL_Window *gWindow;
static SDL_Renderer *gRenderer;
static SDL_Texture *gTexture;
static Ppu ppu;
static Apu apu[POLYPHONY];
static Mpu mpu;
static Device *devscreen, *devmouse, *devctrl, *devmidi, *devaudio0;

Uint8 zoom = 0, debug = 0, reqdraw = 0;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

static void
audio_callback(void *u, Uint8 *stream, int len)
{
	int i;
	Sint16 *samples = (Sint16 *)stream;
	SDL_memset(stream, 0, len);
	for(i = 0; i < POLYPHONY; ++i)
		apu_render(&apu[i], samples, samples + len / 2);
	(void)u;
}

void
redraw(Uint32 *dst, Uxn *u)
{
	drawppu(&ppu);
	if(debug)
		drawdebugger(&ppu, u->wst.dat, u->wst.ptr);
	SDL_UpdateTexture(gTexture, NULL, dst, ppu.width * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
	reqdraw = 0;
}

void
toggledebug(Uxn *u)
{
	debug = !debug;
	redraw(ppu.output, u);
}

void
togglezoom(Uxn *u)
{
	zoom = zoom == 3 ? 1 : zoom + 1;
	SDL_SetWindowSize(gWindow, ppu.width * zoom, ppu.height * zoom);
	redraw(ppu.output, u);
}

void
quit(void)
{
	free(ppu.output);
	free(ppu.fg);
	free(ppu.bg);
	SDL_UnlockAudioDevice(audio_id);
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
	SDL_AudioSpec as;
	if(!initppu(&ppu, 48, 32, 16))
		return error("PPU", "Init failure");
	if(!initmpu(&mpu, 1))
		return error("MPU", "Init failure");
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Uxn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ppu.width * zoom, ppu.height * zoom, SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ppu.width, ppu.height);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);
	SDL_zero(as);
	as.freq = SAMPLE_FREQUENCY;
	as.format = AUDIO_S16;
	as.channels = 2;
	as.callback = audio_callback;
	as.samples = 512;
	as.userdata = NULL;
	audio_id = SDL_OpenAudioDevice(NULL, 0, &as, NULL, 0);
	if(!audio_id)
		return error("Audio", SDL_GetError());
	SDL_PauseAudioDevice(audio_id, 0);
	return 1;
}

void
domouse(SDL_Event *event)
{
	Uint8 flag = 0x00;
	Uint16 x = clamp(event->motion.x / zoom - ppu.pad, 0, ppu.hor * 8 - 1);
	Uint16 y = clamp(event->motion.y / zoom - ppu.pad, 0, ppu.ver * 8 - 1);
	mempoke16(devmouse->dat, 0x2, x);
	mempoke16(devmouse->dat, 0x4, y);
	devmouse->dat[7] = 0x00;
	switch(event->button.button) {
	case SDL_BUTTON_LEFT: flag = 0x01; break;
	case SDL_BUTTON_RIGHT: flag = 0x10; break;
	}
	switch(event->type) {
	case SDL_MOUSEBUTTONDOWN:
		devmouse->dat[6] |= flag;
		if(flag == 0x10 && (devmouse->dat[6] & 0x01))
			devmouse->dat[7] = 0x01;
		if(flag == 0x01 && (devmouse->dat[6] & 0x10))
			devmouse->dat[7] = 0x10;
		break;
	case SDL_MOUSEBUTTONUP:
		devmouse->dat[6] &= (~flag);
		break;
	}
}

void
doctrl(Uxn *u, SDL_Event *event, int z)
{
	Uint8 flag = 0x00;
	if(z && event->key.keysym.sym == SDLK_h) {
		if(SDL_GetModState() & KMOD_LCTRL)
			toggledebug(u);
		if(SDL_GetModState() & KMOD_LALT)
			togglezoom(u);
	}
	switch(event->key.keysym.sym) {
	case SDLK_LCTRL: flag = 0x01; break;
	case SDLK_LALT: flag = 0x02; break;
	case SDLK_LSHIFT: flag = 0x04; break;
	case SDLK_ESCAPE: flag = 0x08; break;
	case SDLK_UP: flag = 0x10; break;
	case SDLK_DOWN: flag = 0x20; break;
	case SDLK_LEFT: flag = 0x40; break;
	case SDLK_RIGHT: flag = 0x80; break;
	}
	if(flag && z)
		devctrl->dat[2] |= flag;
	else if(flag)
		devctrl->dat[2] &= (~flag);
	if(z && event->key.keysym.sym < 20)
		devctrl->dat[3] = event->key.keysym.sym;
}

#pragma mark - Devices

void
system_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(!w) {
		d->dat[0x2] = d->u->wst.ptr;
		d->dat[0x3] = d->u->rst.ptr;
	} else {
		putcolors(&ppu, &d->dat[0x8]);
		reqdraw = 1;
	}
	(void)b0;
}

void
console_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(!w) return;
	switch(b0) {
	case 0x8: printf("%c", d->dat[0x8]); break;
	case 0x9: printf("0x%02x", d->dat[0x9]); break;
	case 0xb: printf("0x%04x", mempeek16(d->dat, 0xa)); break;
	case 0xd: printf("%s", &d->mem[mempeek16(d->dat, 0xc)]); break;
	}
	fflush(stdout);
}

void
screen_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(w && b0 == 0xe) {
		Uint16 x = mempeek16(d->dat, 0x8);
		Uint16 y = mempeek16(d->dat, 0xa);
		Uint8 *addr = &d->mem[mempeek16(d->dat, 0xc)];
		Uint8 *layer = d->dat[0xe] >> 4 & 0x1 ? ppu.fg : ppu.bg;
		Uint8 mode = d->dat[0xe] >> 5;
		if(!mode)
			putpixel(&ppu, layer, x, y, d->dat[0xe] & 0x3);
		else if(mode-- & 0x1)
			puticn(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);
		else
			putchr(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);

		reqdraw = 1;
	}
}

void
file_talk(Device *d, Uint8 b0, Uint8 w)
{
	Uint8 read = b0 == 0xd;
	if(w && (read || b0 == 0xf)) {
		char *name = (char *)&d->mem[mempeek16(d->dat, 0x8)];
		Uint16 result = 0, length = mempeek16(d->dat, 0xa);
		Uint16 offset = mempeek16(d->dat, 0x4);
		Uint16 addr = mempeek16(d->dat, b0 - 1);
		FILE *f = fopen(name, read ? "r" : (offset ? "a" : "w"));
		if(f) {
			printf("%s %04x %s %s: ", read ? "Loading" : "Saving", addr, read ? "from" : "to", name);
			if(fseek(f, offset, SEEK_SET) != -1)
				result = read ? fread(&d->mem[addr], 1, length, f) : fwrite(&d->mem[addr], 1, length, f);
			printf("%04x bytes\n", result);
			fclose(f);
		}
		mempoke16(d->dat, 0x2, result);
	}
}

static void
audio_talk(Device *d, Uint8 b0, Uint8 w)
{
	Apu *c = &apu[d - devaudio0];
	if(!w) {
		if(b0 == 0x2)
			mempoke16(d->dat, 0x2, c->i);
		else if(b0 == 0x4)
			d->dat[0x4] = apu_get_vu(c);
	} else if(b0 == 0xf) {
		SDL_LockAudioDevice(audio_id);
		c->len = mempeek16(d->dat, 0xa);
		c->addr = &d->mem[mempeek16(d->dat, 0xc)];
		c->volume[0] = d->dat[0xe] >> 4;
		c->volume[1] = d->dat[0xe] & 0xf;
		c->repeat = !(d->dat[0xf] & 0x80);
		apu_start(c, mempeek16(d->dat, 0x8), d->dat[0xf] & 0x7f);
		SDL_UnlockAudioDevice(audio_id);
	}
}

void
datetime_talk(Device *d, Uint8 b0, Uint8 w)
{
	time_t seconds = time(NULL);
	struct tm *t = localtime(&seconds);
	t->tm_year += 1900;
	mempoke16(d->dat, 0x0, t->tm_year);
	d->dat[0x2] = t->tm_mon;
	d->dat[0x3] = t->tm_mday;
	d->dat[0x4] = t->tm_hour;
	d->dat[0x5] = t->tm_min;
	d->dat[0x6] = t->tm_sec;
	d->dat[0x7] = t->tm_wday;
	mempoke16(d->dat, 0x08, t->tm_yday);
	d->dat[0xa] = t->tm_isdst;
	(void)b0;
	(void)w;
}

void
midi_talk(Device *d, Uint8 b0, Uint8 w)
{
	(void)d;
	(void)b0;
	(void)w;
}

void
nil_talk(Device *d, Uint8 b0, Uint8 w)
{
	(void)d;
	(void)b0;
	(void)w;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	evaluxn(u, 0x0100);
	redraw(ppu.output, u);
	while(1) {
		int i;
		SDL_Event event;
		double elapsed, start = SDL_GetPerformanceCounter();
		while(SDL_PollEvent(&event) != 0) {
			switch(event.type) {
			case SDL_QUIT:
				quit();
				break;
			case SDL_TEXTINPUT:
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(event.text.text[0] >= ' ' || event.text.text[0] <= '~')
					devctrl->dat[3] = event.text.text[0];
				doctrl(u, &event, event.type == SDL_KEYDOWN);
				evaluxn(u, mempeek16(devctrl->dat, 0));
				devctrl->dat[3] = 0;
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				domouse(&event);
				evaluxn(u, mempeek16(devmouse->dat, 0));
				break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(ppu.output, u);
				break;
			}
		}
		listenmpu(&mpu);
		for(i = 0; i < mpu.queue; ++i) {
			devmidi->dat[2] = mpu.events[i].message;
			devmidi->dat[3] = mpu.events[i].message >> 8;
			devmidi->dat[4] = mpu.events[i].message >> 16;
			evaluxn(u, mempeek16(devmidi->dat, 0));
		}
		evaluxn(u, mempeek16(devscreen->dat, 0));
		if(reqdraw)
			redraw(ppu.output, u);
		elapsed = (SDL_GetPerformanceCounter() - start) / (double)SDL_GetPerformanceFrequency() * 1000.0f;
		SDL_Delay(clamp(16.666f - elapsed, 0, 1000));
	}
	return 1;
}

int
main(int argc, char **argv)
{
	Uxn u;
	zoom = 2;

	if(argc < 2)
		return error("Input", "Missing");
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, argv[1]))
		return error("Load", "Failed");
	if(!init())
		return error("Init", "Failed");

	portuxn(&u, 0x0, "system", system_talk);
	portuxn(&u, 0x1, "console", console_talk);
	devscreen = portuxn(&u, 0x2, "screen", screen_talk);
	devaudio0 = portuxn(&u, 0x3, "audio0", audio_talk);
	portuxn(&u, 0x4, "audio1", audio_talk);
	portuxn(&u, 0x5, "audio2", audio_talk);
	portuxn(&u, 0x6, "audio3", audio_talk);
	devmidi = portuxn(&u, 0x7, "midi", midi_talk);
	devctrl = portuxn(&u, 0x8, "controller", nil_talk);
	devmouse = portuxn(&u, 0x9, "mouse", nil_talk);
	portuxn(&u, 0xa, "file", file_talk);
	portuxn(&u, 0xb, "datetime", datetime_talk);
	portuxn(&u, 0xc, "---", nil_talk);
	portuxn(&u, 0xd, "---", nil_talk);
	portuxn(&u, 0xe, "---", nil_talk);
	portuxn(&u, 0xf, "---", nil_talk);

	/* Write screen size to dev/screen */
	mempoke16(devscreen->dat, 2, ppu.hor * 8);
	mempoke16(devscreen->dat, 4, ppu.ver * 8);

	start(&u);
	quit();
	return 0;
}
