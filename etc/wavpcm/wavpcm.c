#include <stdio.h>
#include <math.h>

/* 
Copyright (c) 2020 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define PI 3.14159265358979323846
#define SAMPLES 256
#define RATE 1

typedef unsigned char Uint8;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

Uint8
sinw(int i)
{
	return 0x7f * sin(i * RATE * 2 * PI / SAMPLES);
}

Uint8
triw(int i)
{
	if(i < 0x40)
		return i * 2;
	if(i >= 0xc0)
		return (i - 0xc0) * 2 - 0x7f;
	return 0x7f - (i - 0x40) * 2;
}

Uint8
sqrw(int i)
{
	return ((i * RATE) % 0xff) < 0x80 ? 0x7f : 0x80;
}

int
main()
{
	int i;
	printf("%d:\n\n", SAMPLES);
	for(i = 0; i < SAMPLES; ++i) {
		if(i % 0x10 == 0)
			printf("\n");
		else if(i % 2 == 0)
			printf(" ");
		printf("%02x", (triw(i) + sinw(i)) / 2);
	}
	printf("\n\n");
	return 0;
}
