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
#include "apu.h"

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

static void
render_note(Apu *apu, Uxn *u, int note_i, Sint16 *samples, int n_samples)
{
	int i;
	Note *note = &apu->notes[note_i];
	while(n_samples--) {
		Sint32 sample = 1;
		for(i = 0; i < 2; ++i) {
			WaveformGenerator *wv = &note->wv[i];
			apu->queue = &wv->queue;
			wv->count += wv->advance;
			while(wv->count > wv->period) {
				wv->count -= wv->period;
				wv->start_value = wv->end_value;
				if(apu->queue->i == apu->queue->n) {
					apu->queue->i = apu->queue->n = 0;
					if(!apu->queue->finishes) {
						*apu->channel_ptr = note_i;
						evaluxn(u, wv->vector);
					}
				}
				if(!apu->queue->n) {
					note->playing = 0;
					return;
				}
				wv->end_value = (Sint16)apu->queue->dat[apu->queue->i++];
				wv->period = (30 << 4) * apu->queue->dat[apu->queue->i++];
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

void
apu_render(Apu *apu, Uxn *u, Sint16 *samples, int n_samples)
{
	int i;
	for(i = 0; i < n_samples * 2; ++i)
		samples[i] = 0;
	for(i = 0; i < apu->n_notes; ++i)
		if(apu->notes[i].playing) render_note(apu, u, i, samples, n_samples);
	apu->queue = NULL;
}

void
apu_play_note(Note *note, Uint16 wave_vector, Uint16 envelope_vector, Uint8 volume, Uint8 pitch, Uint8 impl)
{
	int i;
	if(pitch >= 108 || impl == 0) return;
	note->playing = 1;
	note->impl = impl;
	for(i = 0; i < 2; ++i) {
		note->volume[i] = 0xf & (volume >> 4 * (1 - i));
		note->wv[i].count = note->wv[i].period = 0;
		note->wv[i].end_value = 0x8000 * (1 - i);
		note->wv[i].queue.n = note->wv[i].queue.i = 0;
		note->wv[i].queue.finishes = 0;
		note->wv[i].queue.is_envelope = i;
	}
	note->wv[0].vector = wave_vector;
	note->wv[0].advance = note_advances[pitch % 12] >> (8 - pitch / 12);
	note->wv[1].vector = envelope_vector;
	note->wv[1].advance = (30 << 20) / SAMPLE_FREQUENCY;
}
