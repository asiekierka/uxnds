#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <stdio.h>
#include <time.h>
#include "uxn.h"
#include "devices/ppu.h"
#include "devices/apu.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define AUDIO_BUFFER_SIZE 2048

static C3D_RenderTarget *topLeft, *topRight, *bottom;
static C2D_Image gpuBg, gpuFg;
static C3D_Tex texBg, texFg;
static const Tex3DS_SubTexture gpuFbSub = {
	320, 240, 0.0f, 1.0f, (320.0f/512.0f), (16/256.0f)
};

static bool soundFillBlock;
static ndspWaveBuf soundBuffer[2];
static u8 *soundData;
static LightLock soundLock;

static Ppu ppu;
static Apu apu[POLYPHONY];
static Device *devscreen, *devmouse, *devctrl, *devaudio0;

#define PAD 0

#define REQDRAW_BG 1
#define REQDRAW_FG 2
#define REQDRAW_DISPLAY 4
#define REQDRAW_ALL 7

Uint8 dispswap = 0, debug = 0, reqdraw = 0;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

int
error(char *msg, const char *err)
{
	consoleInit(GFX_BOTTOM, NULL);
	iprintf("Error %s: %s\n", msg, err);
	gfxSwapBuffers();
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break;
		gspWaitForVBlank();
	}
	exit(0);
	return 0;
}

static void
audio_callback(void *u)
{
	if (soundBuffer[soundFillBlock].status == NDSP_WBUF_DONE) {
		int i;
		Sint16 *samples = (Sint16 *) soundBuffer[soundFillBlock].data_vaddr;
		memset(samples, 0, AUDIO_BUFFER_SIZE * 4);
		LightLock_Lock(&soundLock);
		for(i = 0; i < POLYPHONY; ++i)
			apu_render(&apu[i], samples, samples + (AUDIO_BUFFER_SIZE * 2));
		LightLock_Unlock(&soundLock);
		DSP_FlushDataCache(samples, AUDIO_BUFFER_SIZE * 4);
		ndspChnWaveBufAdd(0, &soundBuffer[soundFillBlock]);
		soundFillBlock = !soundFillBlock;
	}
}

void
redraw(Uxn *u)
{
	C2D_DrawParams drawParams;
	float slider = osGet3DSliderState();
	int x_offset_bg = (int) (slider * 7.0f);
	int x_offset_fg = (slider > 0.0f) ? -1 : 0;

	if(debug) {
		drawdebugger(&ppu, u->wst.dat, u->wst.ptr);
		reqdraw |= REQDRAW_BG;
	}
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	if (reqdraw & REQDRAW_BG) {
		C3D_SyncDisplayTransfer(
			ppu.bg.pixels, GX_BUFFER_DIM(PPU_TEX_WIDTH, PPU_TEX_HEIGHT),
			texBg.data, GX_BUFFER_DIM(PPU_TEX_WIDTH, PPU_TEX_HEIGHT),
			(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) |
			GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO) |
			GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
			GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8))
		);
	}
	if (reqdraw & REQDRAW_FG) {
		C3D_SyncDisplayTransfer(
			ppu.fg.pixels, GX_BUFFER_DIM(PPU_TEX_WIDTH, PPU_TEX_HEIGHT),
			texFg.data, GX_BUFFER_DIM(PPU_TEX_WIDTH, PPU_TEX_HEIGHT),
			(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) |
			GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO) |
			GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
			GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8))
		);
	}

	memset(&drawParams, 0, sizeof(drawParams));
	drawParams.pos.y = 0.0f;
	drawParams.pos.w = 320.0f;
	drawParams.pos.h = 240.0f;

	if (!dispswap) {
		C2D_TargetClear(topLeft, C2D_Color32(0, 0, 0, 0));
		C2D_SceneBegin(topLeft);
		drawParams.pos.x = 40.0f - x_offset_bg;
		drawParams.depth = 0.0f;
		C2D_DrawImage(gpuBg, &drawParams, NULL);
		drawParams.pos.x = 40.0f - x_offset_fg;
		drawParams.depth = 1.0f;
		C2D_DrawImage(gpuFg, &drawParams, NULL);

		if (slider > 0.0f) {
			C2D_TargetClear(topRight, C2D_Color32(0, 0, 0, 0));
			C2D_SceneBegin(topRight);
			drawParams.pos.x = 40.0f + x_offset_bg;
			drawParams.depth = 0.0f;
			C2D_DrawImage(gpuBg, &drawParams, NULL);
			drawParams.pos.x = 40.0f + x_offset_fg;
			drawParams.depth = 1.0f;
			C2D_DrawImage(gpuFg, &drawParams, NULL);
		}

		C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 0));
		C2D_SceneBegin(bottom);
	} else {
		C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 0));
		C2D_SceneBegin(bottom);
		drawParams.pos.x = 0.0f;
		drawParams.depth = 0.0f;
		C2D_DrawImage(gpuBg, &drawParams, NULL);
		drawParams.depth = 1.0f;
		C2D_DrawImage(gpuFg, &drawParams, NULL);

		// TODO: keyboard
		C2D_TargetClear(topLeft, C2D_Color32(0, 0, 0, 0));
		C2D_SceneBegin(topLeft);
	}

	C3D_FrameEnd(0);
	reqdraw = 0;
}

void
toggledebug(Uxn *u)
{
	debug = !debug;
	redraw(u);
}

void
quit(void)
{
	linearFree(ppu.fg.pixels);
	linearFree(ppu.bg.pixels);
	C3D_TexDelete(&texFg);
	C3D_TexDelete(&texBg);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	exit(0);
}

int
init(void)
{
	osSetSpeedupEnable(1);
	// PPU
	if(!initppu(&ppu, 40, 30))
		return error("PPU", "Init failure");
	gfxInitDefault();
	gfxSet3D(true);
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(4096);
	C2D_Prepare();
	topLeft = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	topRight = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	gpuBg.tex = &texBg;
	gpuBg.subtex = &gpuFbSub;
	if (!C3D_TexInitVRAM(&texBg, PPU_TEX_WIDTH, PPU_TEX_HEIGHT, GPU_RGBA8))
		return error("PPU", "Could not allocate BG texture");
	gpuFg.tex = &texFg;
	gpuFg.subtex = &gpuFbSub;
	if (!C3D_TexInitVRAM(&texFg, PPU_TEX_WIDTH, PPU_TEX_HEIGHT, GPU_RGBA8))
		return error("PPU", "Could not allocate FG texture");
	// APU
	float soundMix[12];
	memset(soundMix, 0, sizeof(soundMix));
	soundMix[0] = soundMix[1] = 1.0f;
	soundData = (u8*) linearAlloc(AUDIO_BUFFER_SIZE * 8);
	memset(soundData, 0, AUDIO_BUFFER_SIZE * 8);
	LightLock_Init(&soundLock);
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnReset(0);
	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, SAMPLE_FREQUENCY);
	ndspChnSetFormat(0, NDSP_CHANNELS(2) | NDSP_ENCODING(NDSP_ENCODING_PCM16));
	ndspChnSetMix(0, soundMix);
	ndspSetOutputCount(1);
	ndspSetMasterVol(1.0f);
	ndspSetCallback(audio_callback, soundBuffer);

	memset(soundBuffer, 0, sizeof(soundBuffer));
	soundBuffer[0].data_vaddr = &soundData[0];
	soundBuffer[0].nsamples = AUDIO_BUFFER_SIZE;
	soundBuffer[1].data_vaddr = &soundData[AUDIO_BUFFER_SIZE * 4];
	soundBuffer[1].nsamples = AUDIO_BUFFER_SIZE;

	DSP_FlushDataCache(soundData, AUDIO_BUFFER_SIZE * 8);

	ndspChnWaveBufAdd(0, &soundBuffer[0]);
	ndspChnWaveBufAdd(0, &soundBuffer[1]);
	return 1;
}

void
doctrl(Uxn *u)
{
	// TODO: keyboard
	bool changed = false;
	u8 old_flags = devctrl->dat[2];
	// int key = dispswap ? -1 : keyboardUpdate();
	int key = -1;

	int held = hidKeysDown() | hidKeysHeld();
	devctrl->dat[2] = (held & 0x0F)
		| ((held & KEY_UP) ? 0x10 : 0)
		| ((held & KEY_DOWN) ? 0x20 : 0)
		| ((held & KEY_RIGHT) ? 0x80 : 0)
		| ((held & KEY_LEFT) ? 0x40 : 0);

	if (key > 0) {
		devctrl->dat[3] = key;
		changed = true;
	}

	changed |= old_flags != devctrl->dat[2];
	if (changed) {
		evaluxn(u, mempeek16(devctrl->dat, 0));
		devctrl->dat[3] = 0;
	}
}

static Uint8 last_dispswap = 0;
static touchPosition tpos;

void
domouse(Uxn *u)
{
	bool firstTouch;

	if (hidKeysDown() & (KEY_L | KEY_R)) {
		dispswap ^= 1;
		gfxSet3D(!dispswap);
		reqdraw |= REQDRAW_DISPLAY;
	}

	if (last_dispswap || (hidKeysUp() & KEY_TOUCH)) {
		mempoke16(devmouse->dat, 0x2, tpos.px);
		mempoke16(devmouse->dat, 0x4, tpos.py);
		devmouse->dat[6] = 0x00;
		devmouse->dat[7] = 0x00;
		evaluxn(u, mempeek16(devmouse->dat, 0));
		last_dispswap = 0;
	} else if (dispswap && ((hidKeysDown() | hidKeysHeld()) & KEY_TOUCH)) {
		firstTouch = (hidKeysDown() & KEY_TOUCH);
		hidTouchRead(&tpos);
		if (firstTouch
			|| mempeek16(devmouse->dat, 0x2) != tpos.px
			|| mempeek16(devmouse->dat, 0x4) != tpos.py)
		{
			mempoke16(devmouse->dat, 0x2, tpos.px);
			mempoke16(devmouse->dat, 0x4, tpos.py);
			devmouse->dat[6] = 0x01;
			devmouse->dat[7] = 0x00;
			evaluxn(u, mempeek16(devmouse->dat, 0));
			last_dispswap = 1;
		}
	}
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
		reqdraw = REQDRAW_ALL;
	}
	(void)b0;
}

void
console_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(!w) return;
	switch(b0) {
	case 0x8: dprintf("%c", d->dat[0x8]); break;
	case 0x9: dprintf("0x%02x", d->dat[0x9]); break;
	case 0xb: dprintf("0x%04x", mempeek16(d->dat, 0xa)); break;
	case 0xd: dprintf("%s", &d->mem[mempeek16(d->dat, 0xc)]); break;
	}
	fflush(stdout);
}

void
screen_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(w && b0 == 0xe) {
		bool layer_fg = (d->dat[0xe] >> 4) & 0x1;
		Uint16 x = mempeek16(d->dat, 0x8);
		Uint16 y = mempeek16(d->dat, 0xa);
		Uint8 *addr = &d->mem[mempeek16(d->dat, 0xc)];
		Layer *layer = layer_fg ? &ppu.fg : &ppu.bg;
		Uint8 mode = d->dat[0xe] >> 5;
		if(!mode)
			putpixel(&ppu, layer, x, y, d->dat[0xe] & 0x3);
		else if(mode-- & 0x1)
			puticn(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);
		else
			putchr(&ppu, layer, x, y, addr, d->dat[0xe] & 0xf, mode & 0x2, mode & 0x4);

		reqdraw |= 1 + layer_fg;
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
		LightLock_Lock(&soundLock);
		c->len = mempeek16(d->dat, 0xa);
		c->addr = &d->mem[mempeek16(d->dat, 0xc)];
		c->volume[0] = d->dat[0xe] >> 4;
		c->volume[1] = d->dat[0xe] & 0xf;
		c->repeat = !(d->dat[0xf] & 0x80);
		apu_start(c, mempeek16(d->dat, 0x8), d->dat[0xf] & 0x7f);
		LightLock_Unlock(&soundLock);
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
	redraw(u);
	while(aptMainLoop()) {
		hidScanInput();
		if ((hidKeysHeld() &
			(KEY_L | KEY_R | KEY_START | KEY_SELECT))
			== (KEY_L | KEY_R | KEY_START | KEY_SELECT)) {
			break;
		}
		doctrl(u);
		domouse(u);
		evaluxn(u, mempeek16(devscreen->dat, 0));
		redraw(u);
	}
	return 1;
}

static Uxn u;

static bool
fexists(const char *name)
{
	FILE *f = fopen(name, "rb");
	if (f != NULL) {
		fclose(f);
		return true;
	} else {
		return false;
	}
}

int
main(int argc, char **argv)
{
	const char *bootpath = "boot.rom";
	bool check_existing = true;

	if(!init())
		return error("Init", "Failed");
	if(argc >= 2 && argv != NULL && argv[1] != NULL)
		bootpath = argv[1];
	else if (argc < 1 || argv == NULL || argv[0] == NULL || argv[0][0] == 0)
		check_existing = false;
	if (!check_existing || !fexists(bootpath)) {
		chdir("/3ds/uxn");
		if (!fexists(bootpath)) {
			chdir("/uxn");
		}
	}
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, bootpath))
		return error("Load", "Failed");

	portuxn(&u, 0x0, "system", system_talk);
	portuxn(&u, 0x1, "console", console_talk);
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
