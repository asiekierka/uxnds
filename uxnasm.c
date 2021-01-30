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
	int len;
	Uint8 data[PRGLEN];
} Program;

char labels[256][16];

char opcodes[][4] = {
	"BRK",
	"LIT",
	"---",
	"POP",
	"DUP",
	"SWP",
	"OVR",
	"ROT",
	/* */
	"JMP",
	"JSR",
	"JEQ",
	"RTS",
	"EQU",
	"NEQ",
	"LTH",
	"GTH",
	/* */};

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
sihx(char *s)
{
	int i = 0;
	char c;
	while((c = s[i++]))
		if(!(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'F'))
			return 0;
	return 1;
}

int
shex(char *s) /* string to num */
{
	int n = 0, i = 0;
	char c;
	while((c = s[i++]))
		if(c >= '0' && c <= '9')
			n = n * 16 + (c - '0');
		else if(c >= 'A' && c <= 'F')
			n = n * 16 + 10 + (c - 'A');
	return n;
}

#pragma mark - Parser

void
addprg(Uint8 hex)
{
	p.data[p.len++] = hex;
}

void
addlabel(char *id, Uint8 addr)
{
	printf("new label: %s=%02x\n", id, addr);
}

void
addconst(char *id, Uint8 value)
{
	printf("new const: %s=%02x\n", id, value);
}

Uint8
findop(char *s)
{
	int i;
	for(i = 0; i < 16; ++i)
		if(scmp(opcodes[i], s))
			return i;
	return 0;
}

int
comment(char *w, int *skip)
{
	if(w[0] == '>') {
		*skip = 0;
		return 1;
	}
	if(w[0] == '<') *skip = 1;
	if(*skip) return 1;
	return 0;
}

void
pass1(FILE *f)
{
	int skip = 0;
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		if(comment(word, &skip))
			continue;
	}
	rewind(f);
}

void
pass2(FILE *f)
{
	int skip = 0;
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		Uint8 op;
		suca(word);
		if(comment(word, &skip)) continue;
		if(word[0] == ']') continue;
		if(word[0] == '+') {
			addprg(0x01);
			addprg(1);
			addprg(shex(word + 1));
		} else if(word[0] == '[') {
			addprg(0x01);
			addprg(shex(word + 1));
		} else if((op = findop(word)))
			addprg(op);
		else if(sihx(word))
			addprg(shex(word));
		else if(scmp(word, "BRK"))
			addprg(0x00);
		else
			printf("unknown: %s\n", word);
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
	pass2(f);
	fwrite(p.data, sizeof(p.data), 1, fopen(argv[2], "wb"));
	fclose(f);
	return 0;
}
