/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "mpu.h"

int
initmpu(Mpu *m, Uint8 device)
{
	int i;
	Pm_Initialize();
	for(i = 0; i < Pm_CountDevices(); ++i)
		printf("Device #%d -> %s%s\n",
			i,
			Pm_GetDeviceInfo(i)->name,
			i == device ? "[x]" : "[ ]");
	Pm_OpenInput(&m->midi, device, NULL, 128, 0, NULL);
	m->queue = 0;
	m->error = pmNoError;
	return 1;
}

void
listenmpu(Mpu *m)
{
	const int result = Pm_Read(m->midi, m->events, 32);
	if(result < 0) {
		m->error = (PmError)result;
		m->queue = 0;
		return;
	}
	m->queue = result;
}
