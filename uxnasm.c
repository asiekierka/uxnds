#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define PRGLEN 256

typedef unsigned char Uint8;
typedef unsigned short Uint16;

typedef struct {
	int ptr;
	Uint16 data[PRGLEN];
} Program;

Program p;

#pragma mark - Helpers

int
scmp(char *a, char *b) /* string compare */
{
	int i = 0;
	while(a[i] == b[i])
		if(!a[i++])
			return 1;
	return 0;
}

int
shex(char *s) /* string to num */
{
	int n = 0, i = 0;
	char c;
	while((c = s[i++]))
		if(c >= '0' && c <= '9')
			n = n * 16 + (c - '0');
		else if(c >= 'a' && c <= 'f')
			n = n * 16 + 10 + (c - 'a');
	return n;
}

#pragma mark - Helpers

Uint8
getopcode(char *s)
{
	if(scmp(s, "add")) {
		return 0x01;
	}
	return 0;
}

void
pass1(FILE *f)
{
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		int lit = 0, val = 0;
		if(word[0] == '#') {
			lit = 0;
			val = shex(word + 1);
		} else {
			lit = 1;
			val = getopcode(word);
		}
		printf("#%d -> %s[%02x %02x]\n", p.ptr, word, lit, val);
		p.data[p.ptr++] = (val << 8) + (lit & 0xff);
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
	fwrite(p.data, sizeof(p.data), 1, fopen(argv[2], "wb"));
	fclose(f);
	return 0;
}
