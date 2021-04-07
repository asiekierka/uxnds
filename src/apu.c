#include <SDL2/SDL.h>
#include <stdlib.h>

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"

#define SAMPLE_FREQUENCY 48000

extern SDL_AudioDeviceID audio_id;
int error(char *msg, const char *err);

static Uint32 note_advances[12] = {
	0x82d01286 / (SAMPLE_FREQUENCY / 30), /* C7 */
	0x8a976073 / (SAMPLE_FREQUENCY / 30),
	0x92d5171d / (SAMPLE_FREQUENCY / 30), /* D7 */
	0x9b904100 / (SAMPLE_FREQUENCY / 30),
	0xa4d053c8 / (SAMPLE_FREQUENCY / 30), /* E7 */
	0xae9d36b0 / (SAMPLE_FREQUENCY / 30), /* F7 */
	0xb8ff493e / (SAMPLE_FREQUENCY / 30),
	0xc3ff6a72 / (SAMPLE_FREQUENCY / 30), /* G7 */
	0xcfa70054 / (SAMPLE_FREQUENCY / 30),
	0xdc000000 / (SAMPLE_FREQUENCY / 30), /* A7 */
	0xe914f623 / (SAMPLE_FREQUENCY / 30),
	0xf6f11003 / (SAMPLE_FREQUENCY / 30) /* B7 */
};

typedef struct {
	Uint16 *dat;
	Uint8 i, n, sz, ends;
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

static Note *notes = NULL;
static int n_notes = 0;
static Queue *q;
static Uint16 id_addr;

static void
play_note(Uxn *u, int note_i, Sint16 *samples, int n_samples)
{
	int i;
	Note *note = &notes[note_i];
	while(n_samples--) {
		Sint32 sample = 1;
		for(i = 0; i < 2; ++i) {
			WaveformGenerator *wv = &note->wv[i];
			q = &wv->queue;
			wv->count += wv->advance;
			while(wv->count > wv->period) {
				wv->count -= wv->period;
				wv->start_value = wv->end_value;
				if(q->i == q->n) {
					q->i = q->n = 0;
					if(!q->ends) {
						u->ram.dat[id_addr] = note_i;
						evaluxn(u, wv->vector);
					}
				}
				if(!q->n) {
					note->playing = 0;
					return;
				}
				wv->end_value = (Sint16)q->dat[q->i++];
				wv->period = (30 << 4) * q->dat[q->i++];
			}
			if(wv->period >> 9)
				sample *= wv->start_value + (Sint32)(wv->end_value - wv->start_value) * (Sint32)(wv->count >> 10) / (Sint32)(wv->period >> 10);
			else
				sample *= wv->end_value;
		}
		for(i = 0; i < 2; ++i)
			*(samples++) += sample / 0xf * note->volume[i] / 0x10000;
	}
}

static void
play_all_notes(void *u, Uint8 *stream, int len)
{
	int i;
	SDL_memset(stream, 0, len);
	for(i = 0; i < n_notes; ++i)
		if(notes[i].playing) play_note(u, i, (Sint16 *)stream, len >> 2);
	q = NULL;
}

static Note *
get_note(Uint8 i)
{
	if(i >= n_notes) notes = SDL_realloc(notes, (i + 1) * sizeof(Note));
	while(i >= n_notes) SDL_zero(notes[n_notes++]);
	return &notes[i];
}

static Uint8
audio_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat + ptr;
	int i;
	if(b0 == 0xa) {
		Note *note = get_note(b1);
		note->playing = 1;
		for(i = 0; i < 2; ++i) {
			note->volume[i] = 0xf & (m[0x8] >> 4 * (1 - i));
			note->wv[i].vector = (m[0x0 + i * 2] << 8) + m[0x1 + i * 2];
			note->wv[i].count = note->wv[i].period = 0;
			note->wv[i].end_value = 0;
			note->wv[i].queue.n = note->wv[i].queue.i = 0;
			note->wv[i].queue.ends = 0;
		}
		note->wv[0].advance = note_advances[m[0x9] % 12] >> (8 - m[0x9] / 12);
		note->wv[1].advance = (30 << 20) / SAMPLE_FREQUENCY;
	} else if(b0 == 0xe && q != NULL) {
		if(q->n == q->sz) {
			q->sz = q->sz < 4 ? 4 : q->sz * 2;
			q->dat = SDL_realloc(q->dat, q->sz * sizeof(*q->dat));
		}
		q->dat[q->n++] = (m[0xb] << 8) + m[0xc];
		q->dat[q->n++] = (m[0xd] << 8) + b1;
	} else if(b0 == 0xf && q != NULL) {
		q->ends = 1;
	}
	return b1;
}

int
initapu(Uxn *u, Uint8 id)
{
	SDL_AudioSpec as;
	SDL_zero(as);
	as.freq = SAMPLE_FREQUENCY;
	as.format = AUDIO_S16;
	as.channels = 2;
	as.callback = play_all_notes;
	as.samples = 2048;
	as.userdata = u;
	audio_id = SDL_OpenAudioDevice(NULL, 0, &as, NULL, 0);
	if(!audio_id)
		return error("Audio", SDL_GetError());
	id_addr = portuxn(u, id, "audio", audio_poke)->addr + 0xa;
	SDL_PauseAudioDevice(audio_id, 0);
	return 1;
}
