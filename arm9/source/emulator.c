#include <nds.h>
#include <fat.h>
#include <filesystem.h>
#include <dirent.h>
#include <stdio.h>
#include "uxn.h"
#include "util.h"
#include "nds/apu.h"
#include "nds/fifo.h"
#include "nds_keyboard.h"
#include "nds_ppu.h"
#include "devices/datetime.h"
#include "devices/file.h"
#include "devices/system.h"
#include "emulator_config.h"

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

Uint8 dispswap;
Uint8 debug = 0;

static PrintConsole *mainConsole;
#ifdef DEBUG
#ifdef DEBUG_PROFILE
static PrintConsole profileConsole;
#endif
#endif

#define TICK_RESET_KEYS (KEY_X | KEY_Y)

int prompt_reset(Uxn *u);

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
#ifdef ENABLE_KEYBOARD
	keyboard_exit();
#endif
	exit(0);
}

int
init(void)
{
	if(!nds_initppu(&ppu))
		return error("PPU", "Init failure");
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_RATE | SAMPLE_FREQUENCY);
	fifoSendValue32(UXNDS_FIFO_CHANNEL, UXNDS_FIFO_CMD_SET_ADDR | ((u32) (&apu_samples)));
#ifdef ENABLE_KEYBOARD
	keyboard_init();
#endif
	return 1;
}

#pragma mark - Devices

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
screen_deo(Uint8 *d, Uint8 port)
{
	if(port == 0xe) {
		Uint8 ctrl = d[0xe];
		Uint8 color = ctrl & 0x3;
		Uint16 x = PEEK2(d + 0x8);
		Uint16 y = PEEK2(d + 0xa);
		Uint32 *layer = (ctrl & 0x40) ? ppu.fg : ppu.bg;
		/* fill mode */
		if(ctrl & 0x80) {
			Uint16 x2 = PPU_TILES_WIDTH * 8;
			Uint16 y2 = PPU_TILES_HEIGHT * 8;
			if(ctrl & 0x10) x2 = x, x = 0;
			if(ctrl & 0x20) y2 = y, y = 0;
			nds_ppu_fill(&ppu, layer, x, y, x2, y2, color);
		}
		/* pixel mode */
		else {
			Uint16 x = peek16(d, 0x8);
			Uint16 y = peek16(d, 0xa);
			nds_ppu_pixel(&ppu, layer, x, y, d[0xe] & 0x3);
			if(d[0x6] & 0x1) POKE2(d + 0x8, x + 1); /* auto x+1 */
			if(d[0x6] & 0x2) POKE2(d + 0xa, y + 1); /* auto y+1 */
		}
	} else if(port == 0xf) {
		Uint8 twobpp = d[0xf] & 0x80;
		Uint16 x = peek16(d, 0x8);
		Uint16 y = peek16(d, 0xa);
		Uint32 *layer = d[0xf] & 0x40 ? ppu.fg : ppu.bg;
		Uint16 addr = peek16(d, 0xc);
		Uint8 n = d[0x6] >> 4;
		Uint8 dx = (d[0x6] & 0x01) << 3;
		Uint8 dy = (d[0x6] & 0x02) << 2;
		int flipx = (d[0xf] & 0x10), fx = flipx ? -1 : 1;
		int flipy = (d[0xf] & 0x20), fy = flipy ? -1 : 1;
		Uint16 dyx = dy * fx, dxy = dx * fy;
		Uint16 len = (n + 1) << (3 + twobpp);
		if(addr > (0x10000 - len)) return;
		for (Uint8 i = 0; i <= n; i++) {
			if (twobpp) {
				nds_ppu_2bpp(&ppu, layer, x + dyx * i, y + dxy * i, &u.ram.dat[addr], d[0xf] & 0xf, flipx, flipy);
				addr += (d[0x6] & 0x04) << 2;
			} else {
				nds_ppu_1bpp(&ppu, layer, x + dyx * i, y + dxy * i, &u.ram.dat[addr], d[0xf] & 0xf, flipx, flipy);
				addr += (d[0x6] & 0x04) << 1;
			}
		}
                poke16(d, 0x8, x + dx * fx); /* auto x+dx */
                poke16(d, 0xa, y + dy * fy); /* auto y+dy */
                poke16(d, 0xc, addr);        /* auto addr */
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
audio_deo(int instance_id, Uint8 *d, Uint8 port)
{
	NdsApu *instance = memUncached(&apu[instance_id]);
	if(port == 0xf) {
		Uint16 addr = peek16(d, 0xc);
		instance->len = peek16(d, 0xa);
		if(instance->len > 0x10000 - addr)
			instance->len = 0x10000 - addr;
		instance->addr = &u.ram.dat[addr];
		instance->volume[0] = d[0xe] >> 4;
		instance->volume[1] = d[0xe] & 0xf;
		instance->repeat = !(d[0xf] & 0x80);
		Uint8 detune = d[0x5];
		nds_apu_start(instance, peek16(d, 0x8), d[0xf] & 0x7f, detune);
		fifoSendValue32(UXNDS_FIFO_CHANNEL, (UXNDS_FIFO_CMD_APU0 + ((instance_id) << 28))
			| ((u32) (&apu)));
		// fifoWaitValue32(UXNDS_FIFO_CHANNEL);
		// fifoGetValue32(UXNDS_FIFO_CHANNEL);
	}
}

static Uint8 audio0_dei(Uint8 *d, Uint8 port) { return audio_dei(0, d, port); }
static Uint8 audio1_dei(Uint8 *d, Uint8 port) { return audio_dei(1, d, port); }
static Uint8 audio2_dei(Uint8 *d, Uint8 port) { return audio_dei(2, d, port); }
static Uint8 audio3_dei(Uint8 *d, Uint8 port) { return audio_dei(3, d, port); }
static Uint8 file0_dei(Uint8 *d, Uint8 port) { return file_dei(0, d, port); }
static Uint8 file1_dei(Uint8 *d, Uint8 port) { return file_dei(1, d, port); }
static void audio0_deo(Uint8 *d, Uint8 port) { audio_deo(0, d, port); }
static void audio1_deo(Uint8 *d, Uint8 port) { audio_deo(1, d, port); }
static void audio2_deo(Uint8 *d, Uint8 port) { audio_deo(2, d, port); }
static void audio3_deo(Uint8 *d, Uint8 port) { audio_deo(3, d, port); }
static void file0_deo(Uint8 *d, Uint8 port) { file_deo(0, u.ram.dat, d, port); }
static void file1_deo(Uint8 *d, Uint8 port) { file_deo(1, u.ram.dat, d, port); }

static Uint8 nds_system_dei(Uint8 *d, Uint8 port) { return system_dei(&u, port); }
static void
nds_system_deo(Uint8 *d, Uint8 port)
{
	system_deo(&u, d, port);
	if(port > 0x7 && port < 0xe)
		nds_putcolors(&ppu, &u.dev[0x8]);
}

#pragma mark - Generics

static u8 ctrl_flags = 0;

void
doctrl(Uxn *u)
{
	bool changed = false;
	u8 old_flags = ctrl_flags;
#ifdef ENABLE_KEYBOARD
	int key = dispswap ? -1 : keyboard_update();
#else
	int key = -1;
#endif

	int pressed = keysDown();
	// pressed or held, clear if repeated
	int held = (pressed | keysHeld()) & ~(keysDownRepeat() & ~pressed);

#ifdef ENABLE_TOUCH
	if (pressed & (KEY_L | KEY_R)) {
		lcdSwap();
		if (!dispswap) videoBgDisableSub(3); else videoBgEnableSub(3);
		dispswap ^= 1;
	}
#endif

	ctrl_flags = (held & 0x0F)
#ifdef ENABLE_KEYBOARD
		| (keyboard_is_held(K_CTRL) ? 0x01 : 0)
		| (keyboard_is_held(K_ALT) ? 0x02 : 0)
		| (keyboard_is_held(K_SHIFT) ? 0x04 : 0)
		| ((key == K_HOME) ? 0x08 : 0)
#endif
		| ((held & 0xC0) >> 2)
		| ((held & KEY_RIGHT) ? 0x80 : 0)
		| ((held & KEY_LEFT) ? 0x40 : 0);

	if (key > 0 && key < 128) {
		u->dev[0x83] = key;
		changed = true;
	}
	if (old_flags != ctrl_flags) {
		changed = true;
	}
	if (changed) {
		// clear only changed bits
		u->dev[0x82] = (u->dev[0x82] & ~(old_flags & (~ctrl_flags))) | (ctrl_flags & (~old_flags));
		uxn_eval(u, GETVEC(u->dev + 0x80));
		if (key > 0 && key < 128) {
			u->dev[0x83] = 0;
		}
	}

	if (key == K_SYSTEM) {
		prompt_reset(u);
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

static int
uxn_load_boot(Uxn *u)
{
	if(system_load(u, "nitro:/boot.rom")) {
		chdir("nitro:/");
		return 1;
	}
	if(system_load(u, "boot.rom")) {
		return 1;
	}
	if(system_load(u, "./uxn/boot.rom")) {
		chdir("./uxn");
		return 1;
	}
	if(system_load(u, "./uxn/launcher.rom")) {
		chdir("/uxn");
		return 1;
	}
	if(system_load(u, "/uxn/boot.rom")) {
		chdir("/uxn");
		return 1;
	}
	if(system_load(u, "/uxn/launcher.rom")) {
		chdir("/uxn");
		return 1;
	}
	return 0;
}

int
prompt_reset(Uxn *u)
{
	consoleClear();
	iprintf("\n\n\n\n\n    Would you like to reset?\n\n      [A] - Yes\n      [B] - No\n");
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

	if(!resetuxn())
		return error("Resetting", "Failed");
	if(!uxn_load_boot(u))
		return error("Load", "Failed");
	if(!nds_initppu(&ppu))
		return error("PPU", "Init failure");
#ifdef ENABLE_KEYBOARD
	keyboard_clear();
#endif
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if ((keysDown() | keysHeld()) == 0) break;
	}
	ctrl_flags = 0;
	uxn_eval(u, 0x0100);

	consoleClear();
	return 0;
}

DTCM_BSS
u32 vbl_counter = 0;

static void vblankHandler(void)
{
	vbl_counter++;
}

int
start(Uxn *u)
{
#ifdef DEBUG_PROFILE
	u32 tticks;
#endif
	u32 last_vbl_counter = 0;

	irqSet(IRQ_VBLANK, vblankHandler);
	irqEnable(IRQ_VBLANK);

	uxn_eval(u, 0x0100);
	while(1) {
		if(u->dev[0x0f]) break; // Run ended.
		scanKeys();
#ifdef DEBUG_PROFILE
		int allHeld = keysDown() | keysHeld();
		// X+Y in debugger mode resets tticks_peak
		if ((keysDown() & TICK_RESET_KEYS) && ((allHeld & TICK_RESET_KEYS) == TICK_RESET_KEYS))
			memset(tticks_peak, 0, sizeof(tticks_peak));
		tticks = timer_ticks(0);
#endif
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
		bool req_wait_vblank;
		{
			int oldIME = enterCriticalSection();
			req_wait_vblank = vbl_counter == last_vbl_counter;
			last_vbl_counter = vbl_counter;
			leaveCriticalSection(oldIME);
		}
		if (req_wait_vblank) {
			swiWaitForVBlank();
			last_vbl_counter++;
		}
#ifdef DEBUG_PROFILE
		tticks = timer_ticks(0);
#endif
		nds_copyppu(&ppu);
#ifdef DEBUG_PROFILE
		profiler_ticks(timer_ticks(0) - tticks, 2, "flip");
#endif
	}

	irqDisable(IRQ_VBLANK);
	irqClear(IRQ_VBLANK);

	return 1;
}

DTCM_BSS
Uxn u;

int
main(int argc, char **argv)
{
	defaultExceptionHandler();

	powerOn(POWER_ALL_2D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);

	keysSetRepeat(27, 4);

#ifdef USE_BOTTOM_SCREEN_DEFAULT
	dispswap = 1;
	lcdSwap();
#else
	dispswap = 0;
#endif

	mainConsole = consoleDemoInit();
#ifdef DEBUG
#ifdef DEBUG_PROFILE
	// Timers 0-1 - profiling timers
	TIMER0_DATA = 0;
	TIMER1_DATA = 0;
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;
	TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;

	consoleSetWindow(mainConsole, 0, 0, 32, 24);

	profileConsole = *mainConsole;
	consoleSetWindow(&profileConsole, 0, 11, 32, 4);
#else
	consoleSetWindow(mainConsole, 0, 0, 32, 24);
#endif
#endif
	consoleSelect(mainConsole);

	uxn_register_device(0x0, nds_system_dei, nds_system_deo);
	uxn_register_device(0x1, NULL, console_deo);
	uxn_register_device(0x2, screen_dei, screen_deo);
	uxn_register_device(0x3, audio0_dei, audio0_deo);
	uxn_register_device(0x4, audio1_dei, audio1_deo);
	uxn_register_device(0x5, audio2_dei, audio2_deo);
	uxn_register_device(0x6, audio3_dei, audio3_deo);
	uxn_register_device(0xa, file0_dei, file0_deo);
	uxn_register_device(0xb, file1_dei, file1_deo);
	uxn_register_device(0xc, datetime_dei, NULL);

	if(!uxn_boot())
		return error("Boot", "Failed");
	if(!fatInitDefault())
		return error("FAT init", "Failed");
	nitroFSInit(NULL); // no big deal if this one fails
	if(!uxn_load_boot(&u)) {
                dprintf("Halted: Missing input rom.\n");
		return error("Load", "Failed");
	}
	if(!init())
		return error("Init", "Failed");

	/* Write screen size to dev/screen */
	poke16(u.dev + 0x20, 2, PPU_PIXELS_WIDTH);
	poke16(u.dev + 0x20, 4, PPU_PIXELS_HEIGHT);

	start(&u);
#ifdef DEBUG
	scanKeys();
	while(keysHeld()) scanKeys();
	printf("Press any button to exit");
	while(!keysHeld()) scanKeys();
#endif
	quit();
	return 0;
}
