/*---------------------------------------------------------------------------------

Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.

Based on the devkitARM/libnds examples' default ARM7 core.

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/uxn.h"
#include "../../include/apu.h"
#include "../../include/fifo.h"

static u16 sampling_freq, sampling_bufsize;
static u16 sampling_timer_freq;
static s16 *sampling_addr;
static u8 sampling_pos;
static Apu apus[POLYPHONY];

void apu_handler() {
	memset(sampling_addr, 0, sampling_bufsize * 2);
	memset(sampling_addr + (sampling_bufsize * 2), 0, sampling_bufsize * 2);

	for (int i = 0; i < 4; i++) {
		apu_render(&apus[i], sampling_addr, sampling_addr + (sampling_bufsize * 2), sampling_bufsize);
	}

	if (sampling_pos) {
		sampling_addr -= sampling_bufsize;
		sampling_pos = 0;
	} else {
		sampling_addr += sampling_bufsize;
		sampling_pos = 1;
	}
}

void fifo_handler(u32 cmd, void *unused) {
	switch (cmd & UXNDS_FIFO_CMD_MASK) {
		case UXNDS_FIFO_CMD_SET_RATE:
			// init sound hardware
			powerOn(POWER_SOUND);
			writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
			REG_SOUNDCNT = SOUND_ENABLE | 127;
			sampling_freq = cmd & 0xFFFF;
			sampling_timer_freq = (((BUS_CLOCK >> 1) + (sampling_freq >> 1)) / sampling_freq) ^ 0xFFFF;
			break;
		case UXNDS_FIFO_CMD_SET_ADDR:
			sampling_addr = (s16*) (cmd & ~UXNDS_FIFO_CMD_MASK);
			sampling_pos = 0;
			sampling_bufsize = UXNDS_AUDIO_BUFFER_SIZE;

			SCHANNEL_CR(0) = 0;
			SCHANNEL_TIMER(0) = sampling_timer_freq;
			SCHANNEL_SOURCE(0) = sampling_addr;
			SCHANNEL_LENGTH(0) = sampling_bufsize;

			SCHANNEL_CR(0) = SCHANNEL_ENABLE | SOUND_VOL(96) | SOUND_PAN(0) | SOUND_FORMAT_16BIT | SOUND_REPEAT;

			SCHANNEL_CR(1) = 0;
			SCHANNEL_TIMER(1) = sampling_timer_freq;
			SCHANNEL_SOURCE(1) = sampling_addr + (sampling_bufsize * 2);
			SCHANNEL_LENGTH(1) = sampling_bufsize;

			SCHANNEL_CR(1) = SCHANNEL_ENABLE | SOUND_VOL(96) | SOUND_PAN(127) | SOUND_FORMAT_16BIT | SOUND_REPEAT;

			TIMER_DATA(0) = sampling_timer_freq;
			TIMER_CR(0) = TIMER_IRQ_REQ | TIMER_ENABLE | ClockDivider_1024;
			break;
		case UXNDS_FIFO_CMD_APU0:
		case UXNDS_FIFO_CMD_APU1:
		case UXNDS_FIFO_CMD_APU2:
		case UXNDS_FIFO_CMD_APU3:
			Apu *apus_remote = (Apu*) (cmd & ~UXNDS_FIFO_CMD_MASK);
			int oldIME = enterCriticalSection();
			apus[(cmd >> 28) & 0x03] = apus_remote[(cmd >> 28) & 0x03];
			leaveCriticalSection(oldIME);
			break;
	}
}

void VcountHandler() {
	inputGetAndSend();
}

volatile bool exitflag = false;

void powerButtonCB() {
	exitflag = true;
}

int main() {
	dmaFillWords(0, (void*)0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	powerOn(POWER_SOUND);

	readUserSettings();
	ledBlink(0);

	irqInit();
	initClockIRQ();
	fifoInit();
	touchInit();

	SetYtrigger(80);

	fifoSetValue32Handler(UXNDS_FIFO_CHANNEL, fifo_handler, NULL);
	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_TIMER0, apu_handler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK | IRQ_TIMER0);

	setPowerButtonCB(powerButtonCB);

	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}
