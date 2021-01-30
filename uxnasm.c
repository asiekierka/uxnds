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

typedef struct {
	int ptr;
	Uint8 data[PRGLEN];
} Program;

char opcodes[][4] = {"BRK", "LIT", "DUP", "DRP", "SWP", "SLP", "PSH", "POP", "JMP", "JSR", "RST", "BEQ", "EQU", "NEQ", "LTH", "GTH"};

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

char *
suca(char *s) /* string to uppercase */
{
	int i = 0;
	char c;
	while((c = s[i]))
		s[i++] = c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c;
	return s;
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
	int i;
	for(i = 0; i < 16; ++i)
		if(scmp(opcodes[i], suca(s)))
			return i;
	return 0xff;
}

void
echo(Uint8 *s, Uint8 len, Uint8 ptr, char *name)
{
	int i;
	printf("%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		if(ptr == i)
			printf("[%02x]", s[i]);
		else
			printf(" %02x ", s[i]);
	}
	printf("\n");
}

void
pass1(FILE *f)
{
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		int op = getopcode(word);
		if(op == 0xff)
			op = shex(word);
		p.data[p.ptr++] = op;
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
	echo(p.data, 0x40, 0, "program");
	return 0;
}
