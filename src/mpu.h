#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>

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

typedef struct {
	Uint8 queue;
	PmStream *midi;
	PmEvent events[32];
	PmError error;
} Mpu;

int initmpu(Mpu *m, Uint8 device);
void listenmpu(Mpu *m);
