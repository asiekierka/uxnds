#include "../uxn.h"
#include "audio.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define NOTE_PERIOD (SAMPLE_FREQUENCY * 0x4000 / 11025)
#define ADSR_STEP (SAMPLE_FREQUENCY / 0xf)

typedef struct {
	Uint8 *addr;
	Uint32 count, advance, period, age, a, d, s, r;
	Uint16 i, len;
	Sint8 volume[2];
	Uint8 pitch, repeat;
} UxnAudio;

/* clang-format off */

static Uint32 advances[12] = {
	0x80000, 0x879c8, 0x8facd, 0x9837f, 0xa1451, 0xaadc1,
	0xb504f, 0xbfc88, 0xcb2ff, 0xd7450, 0xe411f, 0xf1a1c
};

static UxnAudio uxn_audio[POLYPHONY];

/* clang-format on */

static Sint32
envelope(UxnAudio *c, Uint32 age)
{
	if(!c->r) return 0x0888;
	if(age < c->a) return 0x0888 * age / c->a;
	if(age < c->d) return 0x0444 * (2 * c->d - c->a - age) / (c->d - c->a);
	if(age < c->s) return 0x0444;
	if(age < c->r) return 0x0444 * (c->r - age) / (c->r - c->s);
	c->advance = 0;
	return 0x0000;
}

int
audio_render(int instance, Sint16 *sample, Sint16 *end)
{
	UxnAudio *c = &uxn_audio[instance];
	Sint32 s;
	if(!c->advance || !c->period) return 0;
	while(sample < end) {
		c->count += c->advance;
		c->i += c->count / c->period;
		c->count %= c->period;
		if(c->i >= c->len) {
			if(!c->repeat) {
				c->advance = 0;
				break;
			}
			c->i %= c->len;
		}
		s = (Sint8)(c->addr[c->i] + 0x80) * envelope(c, c->age++);
		*sample++ += s * c->volume[0] / 0x180;
		*sample++ += s * c->volume[1] / 0x180;
	}
	if(!c->advance) audio_finished_handler(instance);
	return 1;
}

void
audio_start(int instance, Uint8 *d, Uxn *u)
{
	UxnAudio *c = &uxn_audio[instance];
	Uint16 addr, adsr;
	Uint8 pitch;
	PEKDEV(adsr, 0x8);
	PEKDEV(c->len, 0xa);
	PEKDEV(addr, 0xc);
	if(c->len > 0x10000 - addr)
		c->len = 0x10000 - addr;
	c->addr = &u->ram.dat[addr];
	c->volume[0] = d[0xe] >> 4;
	c->volume[1] = d[0xe] & 0xf;
	c->repeat = !(d[0xf] & 0x80);
	pitch = d[0xf] & 0x7f;
	if(pitch < 108 && c->len)
		c->advance = advances[pitch % 12] >> (8 - pitch / 12);
	else {
		c->advance = 0;
		return;
	}
	c->a = ADSR_STEP * (adsr >> 12);
	c->d = ADSR_STEP * (adsr >> 8 & 0xf) + c->a;
	c->s = ADSR_STEP * (adsr >> 4 & 0xf) + c->d;
	c->r = ADSR_STEP * (adsr >> 0 & 0xf) + c->s;
	c->age = 0;
	c->i = 0;
	if(c->len <= 0x100) /* single cycle mode */
		c->period = NOTE_PERIOD * 337 / 2 / c->len;
	else /* sample repeat mode */
		c->period = NOTE_PERIOD;
}

Uint8
audio_get_vu(int instance)
{
	UxnAudio *c = &uxn_audio[instance];
	int i;
	Sint32 sum[2] = {0, 0};
	if(!c->advance || !c->period) return 0;
	for(i = 0; i < 2; i++) {
		if(!c->volume[i]) continue;
		sum[i] = 1 + envelope(c, c->age) * c->volume[i] / 0x800;
		if(sum[i] > 0xf) sum[i] = 0xf;
	}
	return (sum[0] << 4) | sum[1];
}

Uint16
audio_get_position(int instance)
{
	UxnAudio *c = &uxn_audio[instance];
	return c->i;
}
