/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned int Uint32;
typedef signed int Sint32;

#define SAMPLE_FREQUENCY 44100
#define POLYPHONY 4

typedef struct {
	Uint8 *addr;
	Uint32 count, advance, period, age, a, d, s, r;
	Uint16 i, len;
	Sint8 volume[2];
	Uint8 pitch, repeat;
} Apu;

void apu_render(Apu *c, Sint16 *sample, Sint16 *end);
void apu_start(Apu *c, Uint16 adsr, Uint8 pitch);
Uint8 apu_get_vu(Apu *c);
