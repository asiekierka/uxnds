#include <nds.h>
#include <fat.h>
#include <dirent.h>
#include <stdio.h>
#include "uxn.h"
#include "util.h"
#include "nds/apu.h"
#include "nds/fifo.h"
#include "nds_ppu.h"
#include "devices/datetime.h"
#include "devices/file.h"
#include "devices/system.h"

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
static NdsPpu ppu;
static NdsApu apu[POLYPHONY];
static u32 apu_samples[(UXNDS_AUDIO_BUFFER_SIZE * 4) >> 1];

Uint8 dispswap = 0, debug = 0;
char load_filename[129];

static PrintConsole *mainConsole;
#ifdef DEBUG
#ifdef DEBUG_PROFILE
static PrintConsole profileConsole;
#endif
#endif

#define RESET_KEYS (KEY_START | KEY_SELECT)
#define TICK_RESET_KEYS (KEY_X | KEY_Y)

int
error(char *msg, const char *err)
{
	dprintf("Error %s: %s\n", msg, err);
	while(1) {
		swiWaitForVBlank();
	}
}

void
quit(void)
{
	exit(0);
}

int
init(void)
{
	if(!nds_initppu(&ppu))
		return error("PPU", "Init failure");
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_RATE | SAMPLE_FREQUENCY);
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_ADDR | ((u32) (&apu_samples)));
	return 1;
}

#pragma mark - Devices

static int
console_input(Uxn *u, char c)
{
	Uint8 *d = &u->dev[0x10];
	d[0x02] = c;
	return uxn_eval(u, GETVEC(d));
}

static void
console_deo(Uint8 *d, Uint8 port)
{
	if(port >= 0x8) {
		fputc(d[port], stdout);
		fflush(stdout);
	}
}

ITCM_ARM_CODE
static Uint8
screen_dei(Uint8 *d, Uint8 port)
{
        switch(port) {
                case 0x2: return PPU_PIXELS_WIDTH >> 8;
                case 0x3: return PPU_PIXELS_WIDTH & 0xFF;
                case 0x4: return PPU_PIXELS_HEIGHT >> 8;
                case 0x5: return PPU_PIXELS_HEIGHT & 0xFF;
		default: return d[port];
        }
}

ITCM_ARM_CODE
static void
screen_deo(Uint8 *d, Uint8 port, Uxn *u)
{
	if(port == 0xe) {
		Uint16 x = peek16(d, 0x8);
		Uint16 y = peek16(d, 0xa);
		Uint32 *layer = (d[0xe] >> 6) & 0x1 ? ppu.fg : ppu.bg;
		nds_ppu_pixel(&ppu, layer, x, y, d[0xe] & 0x3);
                if(d[0x6] & 0x01) poke16(d, 0x8, x + 1); /* auto x+1 */
                if(d[0x6] & 0x02) poke16(d, 0xa, y + 1); /* auto y+1 */
	} else if(port == 0xf) {
		Uint8 twobpp = d[0xf] >> 7;
		Uint16 x = peek16(d, 0x8);
		Uint16 y = peek16(d, 0xa);
		Uint32 *layer = d[0xf] >> 6 & 0x1 ? ppu.fg : ppu.bg;
		Uint16 addr = peek16(d, 0xc);
		Uint8 n = d[0x6] >> 4;
		Uint8 dx = (d[0x6] & 0x01) << 3;
		Uint8 dy = (d[0x6] & 0x02) << 2;
		Uint16 len = (n + 1) << (3 + twobpp);
		if(addr > (0x10000 - len)) return;
		for (Uint8 i = 0; i <= n; i++) {
			if (twobpp) {
				nds_ppu_2bpp(&ppu, layer, x + dy * i, y + dx * i, &u->ram.dat[addr], d[0xf] & 0xf, d[0xf] >> 0x4 & 0x1, d[0xf] >> 0x5 & 0x1);
				addr += (d[0x6] & 0x04) << 2;
			} else {
				nds_ppu_1bpp(&ppu, layer, x + dy * i, y + dx * i, &u->ram.dat[addr], d[0xf] & 0xf, d[0xf] >> 0x4 & 0x1, d[0xf] >> 0x5 & 0x1);
				addr += (d[0x6] & 0x04) << 1;
			}
		}
                poke16(d, 0x8, x + dx); /* auto x+dx */
                poke16(d, 0xa, y + dy); /* auto y+dy */
                poke16(d, 0xc, addr); /* auto addr */
	}
}

static Uint8
audio_dei(int instance_id, Uint8 *d, Uint8 port)
{
	NdsApu *instance = &apu[instance_id];
	switch(port) {
		case 0x4: return nds_apu_get_vu(instance);
		case 0x2: POKDEV(0x2, instance->i); /* fall through */
		default: return d[port];
	}
}

static void
audio_deo(int instance_id, Uint8 *d, Uint8 port, Uxn *u)
{
	NdsApu *instance = &apu[instance_id];
	if(port == 0xf) {
		instance->len = peek16(d, 0xa);
		instance->addr = &u->ram.dat[peek16(d, 0xc)];
		instance->volume[0] = d[0xe] >> 4;
		instance->volume[1] = d[0xe] & 0xf;
		instance->repeat = !(d[0xf] & 0x80);
		nds_apu_start(instance, peek16(d, 0x8), d[0xf] & 0x7f);
		fifoSendValue32(UXNDS_FIFO_CHANNEL, (UXNDS_FIFO_CMD_APU0 + ((instance_id) << 28))
			| ((u32) (&apu)));
	}
}

static Uint8
emu_dei(Uxn *u, Uint8 addr)
{
	Uint8 p = addr & 0x0f, d = addr & 0xf0;
	switch(d) {
	case 0x20: return screen_dei(&u->dev[d], p);
	case 0x30: return audio_dei(0, &u->dev[d], p);
	case 0x40: return audio_dei(1, &u->dev[d], p);
	case 0x50: return audio_dei(2, &u->dev[d], p);
	case 0x60: return audio_dei(3, &u->dev[d], p);
	case 0xa0: return file_dei(0, &u->dev[d], p);
	case 0xb0: return file_dei(1, &u->dev[d], p);
	case 0xc0: return datetime_dei(&u->dev[d], p);
	}
	return u->dev[addr];
}

static void
emu_deo(Uxn *u, Uint8 addr, Uint8 v)
{
	Uint8 p = addr & 0x0f, d = addr & 0xf0;
	u->dev[addr] = v;
	switch(d) {
	case 0x00:
		system_deo(u, &u->dev[d], p);
		if(p > 0x7 && p < 0xe)
			nds_putcolors(&ppu, &u->dev[0x8]);
		break;
	case 0x10: console_deo(&u->dev[d], p); break;
	case 0x20: screen_deo(&u->dev[d], p, u); break;
	case 0x30: audio_deo(0, &u->dev[d], p, u); break;
	case 0x40: audio_deo(1, &u->dev[d], p, u); break;
	case 0x50: audio_deo(2, &u->dev[d], p, u); break;
	case 0x60: audio_deo(3, &u->dev[d], p, u); break;
	case 0xa0: file_deo(0, u->ram.dat, &u->dev[d], p); break;
	case 0xb0: file_deo(1, u->ram.dat, &u->dev[d], p); break;
	}
}

#pragma mark - Generics

void
doctrl(Uxn *u)
{
	bool changed = false;
	u8 old_flags = u->dev[0x82];
	int key = dispswap ? -1 : keyboardUpdate();

	int pressed = keysDown();
	int held = pressed | keysHeld();

	if (pressed & (KEY_L | KEY_R)) {
		lcdSwap();
		if (dispswap) keyboardShow(); else keyboardHide();
		dispswap ^= 1;
	}

	u->dev[0x82] = (held & 0x0F)
		| ((held & 0xC0) >> 2)
		| ((held & KEY_RIGHT) ? 0x80 : 0)
		| ((held & KEY_LEFT) ? 0x40 : 0);

	switch (key) {
		case DVK_FOLD:
			u->dev[0x83] = 27;
			changed = true;
			break;
		case DVK_UP:
			u->dev[0x82] |= (1 << 4);
			break;
		case DVK_DOWN:
			u->dev[0x82] |= (1 << 5);
			break;
		case DVK_LEFT:
			u->dev[0x82] |= (1 << 6);
			break;
		case DVK_RIGHT:
			u->dev[0x82] |= (1 << 7);
			break;
		case DVK_ENTER:
			u->dev[0x83] = 13;
			changed = true;
			break;
		default:
			if (key > 0) {
				u->dev[0x83] = key;
				changed = true;
			}
			break;
	}

	changed |= old_flags != u->dev[0x82];
	if (changed) {
		uxn_eval(u, GETVEC(u->dev + 0x80));
		u->dev[0x83] = 0;
	}
}

static bool istouching = false;

void
domouse(Uxn *u)
{
	bool changed = false;

	if (dispswap && (keysHeld() & KEY_TOUCH)) {
		if (!istouching) {
			u->dev[0x96] = 0x01;
			istouching = true;
			changed = true;
		}

		touchPosition tpos;
		touchRead(&tpos);
		if (peek16(u->dev + 0x90, 0x2) != tpos.px
			|| peek16(u->dev + 0x90, 0x4) != tpos.py)
		{
			poke16(u->dev + 0x90, 0x2, tpos.px);
			poke16(u->dev + 0x90, 0x4, tpos.py);
			changed = true;
		}
	} else if (istouching) {
		u->dev[0x96] = 0x00;
		istouching = false;
		changed = true;
	}

	if (changed) {
		uxn_eval(u, GETVEC(u->dev + 0x90));
	}
}

#define timer_ticks(tid) (TIMER_DATA((tid)) | (TIMER_DATA((tid)+1) << 16))

#ifdef DEBUG_PROFILE
static Uint32 tticks_peak[3];

void
profiler_ticks(Uint32 tticks, int pos, const char *name)
{
	if (tticks >= 0x80000000)
		tticks = ~tticks;
	if (tticks_peak[pos] < tticks)
		tticks_peak[pos] = tticks;
	consoleSelect(&profileConsole);
	iprintf("\x1b[%d;0H\x1b[0K%s: %d, peak %d\n", pos, name, tticks, tticks_peak[pos]);
	consoleSelect(mainConsole);
}
#endif

int
prompt_reset(Uxn *u)
{
	consoleClear();
	iprintf("Would you like to reset uxnds?\n\n [A] - Yes\n [B] - No\n");
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		int allHeld = keysDown() | keysHeld();
		if (allHeld & KEY_A) {
			consoleClear();
			break;
		} else if (allHeld & KEY_B) {
			consoleClear();
			return 0;
		}
	}

	iprintf("Resetting...\n");

	if(!resetuxn(u))
		return error("Resetting", "Failed");
	if(!system_load(u, load_filename))
		return error("Load", "Failed");
	if(!nds_initppu(&ppu))
		return error("PPU", "Init failure");
	uxn_eval(u, 0x0100);

	consoleClear();
	return 0;
}

int
start(Uxn *u)
{
#ifdef DEBUG_PROFILE
	u32 tticks;
#endif

	uxn_eval(u, 0x0100);
	while(1) {
		if(u->dev[0x0f]) return 1; // Run ended.
		scanKeys();
		int allHeld = keysDown() | keysHeld();
#ifdef DEBUG_PROFILE
		// X+Y in debugger mode resets tticks_peak
		if ((keysDown() & TICK_RESET_KEYS) && ((allHeld & TICK_RESET_KEYS) == TICK_RESET_KEYS))
			memset(tticks_peak, 0, sizeof(tticks_peak));
		tticks = timer_ticks(0);
#endif
		// On the first frame that L+R are held
		if ((keysDown() & RESET_KEYS) && ((allHeld & RESET_KEYS) == RESET_KEYS))
			prompt_reset(u);
		doctrl(u);
		domouse(u);
#ifdef DEBUG_PROFILE
		profiler_ticks(timer_ticks(0) - tticks, 1, "ctrl");
		tticks = timer_ticks(0);
#endif
		uxn_eval(u, GETVEC(u->dev + 0x20));
#ifdef DEBUG_PROFILE
		profiler_ticks(timer_ticks(0) - tticks, 0, "main");
#endif
		swiWaitForVBlank();
#ifdef DEBUG_PROFILE
		tticks = timer_ticks(0);
#endif
		nds_copyppu(&ppu);
#ifdef DEBUG_PROFILE
		profiler_ticks(timer_ticks(0) - tticks, 2, "flip");
#endif
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

	mainConsole = consoleDemoInit();
#ifdef DEBUG
#ifdef DEBUG_PROFILE
	// Timers 0-1 - profiling timers
	TIMER0_DATA = 0;
	TIMER1_DATA = 0;
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;
	TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;

	consoleSetWindow(mainConsole, 0, 0, 32, 11);

	profileConsole = *mainConsole;
	consoleSetWindow(&profileConsole, 0, 11, 32, 4);
#else
	consoleSetWindow(mainConsole, 0, 0, 32, 14);
#endif
#endif
	consoleSelect(mainConsole);

	keyboardDemoInit();

	if(!uxn_boot(&u, (Uint8 *)calloc(0x10000 * RAM_PAGES, sizeof(Uint8)), emu_dei, emu_deo))
		return error("Boot", "Failed");
	if (!fatInitDefault())
		return error("FAT init", "Failed");
	chdir("/uxn");
	if(!system_load(&u, "boot.rom")) {
		if(!system_load(&u, "launcher.rom")) {
	                dprintf("Halted: Missing input rom.\n");
			return error("Load", "Failed");
		}
	}
	if(!init())
		return error("Init", "Failed");

	keyboard = keyboardGetDefault();
	keyboard->scrollSpeed = 0;

	keyboardShow();

	/* Write screen size to dev/screen */
	poke16(u.dev + 0x20, 2, PPU_PIXELS_WIDTH);
	poke16(u.dev + 0x20, 4, PPU_PIXELS_HEIGHT);

	start(&u);
	quit();
	return 0;
}
