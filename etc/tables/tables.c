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

typedef unsigned char Uint8;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

int
main()
{
	int i;
	printf("60 points on a circle128(bytex,bytey):\n\n");
	for(i = 0; i < 60; ++i) {
		double cx = 128, cy = 128, r = 128;
		double pos = (i - 15) % 60;
		double deg = (pos / 60.0) * 360.0;
		double rad = deg * (PI / 180);
		double x = cx + r * cos(rad);
		double y = cy + r * sin(rad);
		if(i > 0 && i % 8 == 0)
			printf("\n");
		printf("%02x%02x ", (Uint8)clamp(x, 0x00, 0xff), (Uint8)clamp(y, 0x00, 0xff));
	}
	printf("\n\n");
	return 0;
}
