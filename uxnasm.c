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
#define LABELIDLEN 32

typedef unsigned char Uint8;

typedef struct {
	int len;
	Uint8 data[PRGLEN];
} Program;

typedef struct {
	Uint8 addr;
	char name[LABELIDLEN];
} Label;

int labelslen;
Label labels[256];

/* clang-format off */

char opcodes[][4] = {
	"BRK", "RTS", "LIT", "POP", "DUP", "SWP", "OVR", "ROT",
	"JMI", "JSI", "JMZ", "JSZ", "EQU", "NEQ", "LTH", "GTH",
	"AND", "ORA", "ROL", "ROR", "ADD", "SUB", "MUL", "DIV"};

/* clang-format on */

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
scpy(char *src, char *dst, int len) /* string copy */
{
	int i = 0;
	while((dst[i] = src[i]) && i < len - 2)
		i++;
	dst[i + 1] = '\0';
	return dst;
}

int
slen(char *s) /* string length */
{
	int i = 0;
	while(s[i] && s[++i])
		;
	return i;
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
		if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
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
		else if(c >= 'a' && c <= 'f')
			n = n * 16 + 10 + (c - 'f');
	return n;
}

#pragma mark - Parser

void
pushprg(Uint8 hex)
{
	p.data[p.len++] = hex;
}

void
pushlabel(Label *l)
{
	pushprg(0x02);
	pushprg(0x01);
	pushprg(l->addr);
}

void
pushliteral(char *w)
{
	int len = slen(w) / 2, value = shex(w);
	pushprg(0x02);
	pushprg(len);
	switch(len) {
	case 1:
		pushprg(value);
		break;
	case 2:
		pushprg(value >> 8);
		pushprg(value);
		break;
	case 3:
		pushprg(value >> 16);
		pushprg(value >> 8);
		pushprg(value);
		break;
	}
}

void
addlabel(char *id, Uint8 addr)
{
	Label *l = &labels[labelslen++];
	scpy(suca(id), l->name, LABELIDLEN);
	l->addr = addr;
	printf("new label: %s=%02x\n", l->name, l->addr);
}

void
addconst(char *id, Uint8 value)
{
	printf("new const: %s=%02x\n", id, value);
}

Label *
findlabel(char *s)
{
	int i;
	for(i = 0; i < labelslen; ++i)
		if(scmp(labels[i].name, s))
			return &labels[i];
	return NULL;
}

Uint8
findop(char *s)
{
	int i;
	for(i = 0; i < 24; ++i)
		if(scmp(opcodes[i], s))
			return i;
	return 0;
}

int
getlength(char *w)
{
	if(findop(w) || scmp(w, "BRK")) return 1;
	if(w[0] == '.') return 3;
	if(w[0] == ':') return 0;
	if(sihx(w)) { return slen(w) / 2 + 2; }
	printf("Unknown length %s\n", w);
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
	int addr = 0;
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		if(comment(word, &skip)) continue;
		if(word[0] == ':') addlabel(word + 1, addr);
		addr += getlength(word);
	}
	rewind(f);
}

void
pass2(FILE *f)
{
	int skip = 0;
	char word[64];
	while(fscanf(f, "%s", word) == 1) {
		Uint8 op = 0;
		Label *l;
		if(word[0] == ':') continue;
		suca(word);
		if(comment(word, &skip)) continue;
		if((op = findop(word)) || scmp(word, "BRK"))
			pushprg(op);
		else if((l = findlabel(word + 1)))
			pushlabel(l);
		else if(sihx(word))
			pushliteral(word);
		else
			printf("Unknown label: %s\n", word);
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
