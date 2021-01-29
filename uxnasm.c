#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define BUFLEN 256

typedef unsigned char Uint8;
typedef unsigned char Uint16;

unsigned short program[BUFLEN];

void
pass1(FILE *f)
{
	int instrid = 0;
	char line[BUFLEN];
	while(fgets(line, BUFLEN, f)) {
		printf("%s\n", line);
	}
}

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	if(argc < 3)
		return error("No input.");
	if(!(f = fopen(argv[1], "r")))
		return error("Missing input.");
	pass1(f);
	fwrite(program, sizeof(program), 1, fopen(argv[2], "wb"));
	return 0;
}
