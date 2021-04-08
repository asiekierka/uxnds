/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned int Uint32;
typedef signed int Sint32;

#define SAMPLE_FREQUENCY 48000

typedef struct {
	Uint16 *dat;
	Uint8 i, n, sz, finishes;
} Queue;

typedef struct {
	Uint32 count, advance, period;
	Uint16 vector;
	Sint16 start_value, end_value;
	Queue queue;
} WaveformGenerator;

typedef struct {
	WaveformGenerator wv[2];
	Sint8 volume[2], playing;
} Note;

typedef struct {
	Queue *queue;
	Note *notes;
	int n_notes;
} Apu;

void apu_render(Apu *apu, Uxn *u, Sint16 *samples, int n_samples);
void apu_play_note(Note *note, Uint16 wave_vector, Uint16 envelope_vector, Uint8 volume, Uint8 pitch);
