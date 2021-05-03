#include <stdio.h>
#include <stdlib.h>

/* 
Copyright (c) 2020 Devine Lu Linvega

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

int
main(int argc, char **argv)
{
	FILE *f;
	Uint8 *buffer;
	Uint16 filelen, i;
	if(argc < 2 || !(f = fopen(argv[1], "rb")))
		return 1;
	fseek(f, 0, SEEK_END);
	filelen = ftell(f);
	rewind(f);
	buffer = (Uint8 *)malloc(filelen * sizeof(Uint8));
	fread(buffer, filelen, 1, f);
	fclose(f);
	for(i = 0; i < filelen; ++i)
		buffer[i] += 0x80;
	printf("\n\n");
	fwrite(buffer, filelen, 1, fopen(argv[2], "wb"));
	return 0;
}
