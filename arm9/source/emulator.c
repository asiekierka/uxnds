#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <time.h>
#include "../../include/uxn.h"
#include "../../include/apu.h"
#include "../../include/fifo.h"
#include "ppu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

DTCM_BSS
static Ppu ppu;
static Apu apu[POLYPHONY];
static u32 apu_samples[(512 * 4) >> 1];
static Device *devscreen, *devctrl, *devmouse, *devaudio0;

Uint8 dispswap = 0, debug = 0;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

int
error(char *msg, const char *err)
{
	dprintf("Error %s: %s\n", msg, err);
	while(1) {}
}

void
quit(void)
{
	exit(0);
}

int
init(void)
{
	if(!initppu(&ppu, 32, 24, 0))
		return error("PPU", "Init failure");
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_RATE | SAMPLE_FREQUENCY);
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_ADDR | ((u32) (&apu_samples)));
	return 1;
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
	}
	(void)b0;
}

#ifdef DEBUG
void
console_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(!w) return;
	switch(b0) {
	case 0x8: iprintf("%c", d->dat[0x8]); break;
	case 0x9: iprintf("0x%02x", d->dat[0x9]); break;
	case 0xb: iprintf("0x%04x", mempeek16(d->dat, 0xa)); break;
	case 0xd: iprintf("%s", &d->mem[mempeek16(d->dat, 0xc)]); break;
	}
	fflush(stdout);
}
#endif

void
screen_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(w && b0 == 0xe) {
		Uint16 x = mempeek16(d->dat, 0x8);
		Uint16 y = mempeek16(d->dat, 0xa);
		Uint8 *addr = &d->mem[mempeek16(d->dat, 0xc)];
		Uint32 *layer = d->dat[0xe] >> 4 & 0x1 ? ppu.fg : ppu.bg;
		Uint8 mode = d->dat[0xe] >> 5;
		if(!mode)
			putpixel(&ppu, layer, x, y, d->dat[0xe] & 0x3);
		else if(mode-- & 0x1)
			puticn(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);
		else
			putchr(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);
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
			dprintf("%s %04x %s %s: ", read ? "Loading" : "Saving", addr, read ? "from" : "to", name);
			if(fseek(f, offset, SEEK_SET) != -1)
				result = read ? fread(&d->mem[addr], 1, length, f) : fwrite(&d->mem[addr], 1, length, f);
			dprintf("%04x bytes\n", result);
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
		c->len = mempeek16(d->dat, 0xa);
		c->addr = &d->mem[mempeek16(d->dat, 0xc)];
		c->volume[0] = d->dat[0xe] >> 4;
		c->volume[1] = d->dat[0xe] & 0xf;
		c->repeat = !(d->dat[0xf] & 0x80);
		apu_start(c, mempeek16(d->dat, 0x8), d->dat[0xf] & 0x7f);
		fifoSendValue32(UXNDS_FIFO_CHANNEL, (UXNDS_FIFO_CMD_APU0 + (d - devaudio0))
			| ((u32) (&apu)));
	}
}

void
nil_talk(Device *d, Uint8 b0, Uint8 w)
{
	(void)d;
	(void)b0;
	(void)w;
}

#pragma mark - Generics

void
doctrl(Uxn *u)
{
	bool changed;
	u8 old_flags = devctrl->dat[2];
	int key = keyboardUpdate();

	int pressed = keysDown();
	int held = pressed | keysHeld();

	if (pressed & (KEY_L | KEY_R)) {
		lcdSwap();
		if (dispswap) keyboardShow(); else keyboardHide();
		dispswap ^= 1;
	}

	devctrl->dat[2] = (held & 0x0F)
		| ((held & 0xC0) >> 2)
		| ((held & KEY_RIGHT) ? 0x80 : 0)
		| ((held & KEY_LEFT) ? 0x40 : 0);
	changed = old_flags != devctrl->dat[2];

	if (key > 0) {
		devctrl->dat[3] = key;
		changed = true;
	}

	if (changed) {
		evaluxn(u, mempeek16(devctrl->dat, 0));
		devctrl->dat[3] = 0;
	}
}

static touchPosition tpos;

void
domouse(Uxn *u)
{
	bool firstTouch;

	if (keysUp() & KEY_TOUCH) {
		mempoke16(devmouse->dat, 0x2, tpos.px);
		mempoke16(devmouse->dat, 0x4, tpos.py);
		devmouse->dat[6] = 0x00;
		devmouse->dat[7] = 0x00;
		evaluxn(u, mempeek16(devmouse->dat, 0));
	} else if ((keysDown() | keysHeld()) & KEY_TOUCH) {
		firstTouch = (keysDown() & KEY_TOUCH);
		touchRead(&tpos);
		if (firstTouch
			|| mempeek16(devmouse->dat, 0x2) != tpos.px
			|| mempeek16(devmouse->dat, 0x4) != tpos.py)
		{
			mempoke16(devmouse->dat, 0x2, tpos.px);
			mempoke16(devmouse->dat, 0x4, tpos.py);
			devmouse->dat[6] = 0x01;
			devmouse->dat[7] = 0x00;
			evaluxn(u, mempeek16(devmouse->dat, 0));
		}
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

#define timer_ticks(tid) (TIMER_DATA((tid)) | (TIMER_DATA((tid)+1) << 16))

int
start(Uxn *u)
{
	u32 tticks;

	evaluxn(u, 0x0100);
	while(1) {
		scanKeys();
		doctrl(u);
		domouse(u);
#ifdef DEBUG_PROFILE
		tticks = timer_ticks(0);
#endif
		evaluxn(u, mempeek16(devscreen->dat, 0));
#ifdef DEBUG_PROFILE
		tticks = timer_ticks(0) - tticks;
		iprintf("%d ticks\n", tticks);
#endif
		swiWaitForVBlank();
		copyppu(&ppu);
	}
	return 1;
}

DTCM_BSS
static Uxn u;

int
main(int argc, char **argv)
{
	Keyboard *keyboard;

	powerOn(POWER_ALL_2D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);

#ifdef DEBUG
	consoleDemoInit();

#ifdef DEBUG_PROFILE
	// Timers 0-1 - profiling timers
	TIMER0_DATA = 0;
	TIMER1_DATA = 0;
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;
	TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;
#endif
#endif

	keyboardDemoInit();

	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if (!fatInitDefault())
		return error("FAT init", "Failed");
	chdir("/uxn");
	if(!loaduxn(&u, "boot.rom"))
		return error("Load", "Failed");
	if(!init())
		return error("Init", "Failed");

	keyboard = keyboardGetDefault();
	keyboard->scrollSpeed = 0;

	keyboardShow();

	portuxn(&u, 0x0, "system", system_talk);
#ifdef DEBUG
	portuxn(&u, 0x1, "console", console_talk);
#else
	portuxn(&u, 0x1, "---", nil_talk);
#endif
	devscreen = portuxn(&u, 0x2, "screen", screen_talk);
	devaudio0 = portuxn(&u, 0x3, "audio0", audio_talk);
	portuxn(&u, 0x4, "audio1", audio_talk);
	portuxn(&u, 0x5, "audio2", audio_talk);
	portuxn(&u, 0x6, "audio3", audio_talk);
	portuxn(&u, 0x7, "---", nil_talk);
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
