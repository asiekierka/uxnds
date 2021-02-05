#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
typedef unsigned short Uint16;

typedef struct {
	int ptr;
	Uint8 data[65536];
} Program;

typedef struct {
	Uint16 addr;
	char name[64];
} Label;

int labelslen;
Label labels[256];

/* clang-format off */

char opcodes[][4] = {
	"BRK", "RTS", "LIT", "POP", "DUP", "SWP", "OVR", "ROT",
	"JMU", "JSU", "JMC", "JSC", "EQU", "NEQ", "GTH", "LTH",
	"AND", "ORA", "ROL", "ROR", "ADD", "SUB", "MUL", "DIV",
	"LDR", "STR", "---", "---", "---", "---", "---", "---"
};

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
sihx(char *s) /* string is hexadecimal */
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
			n = n * 16 + 10 + (c - 'a');
	return n;
}

int
ismarker(char *w)
{
	return w[0] == '[' || w[0] == ']' || w[0] == '{' || w[0] == '}';
}

int
iscomment(char *w, int *skip)
{
	if(w[0] == ')') {
		*skip = 0;
		return 1;
	}
	if(w[0] == '(') *skip = 1;
	if(*skip) return 1;
	return 0;
}

#pragma mark - I/O

void
pushbyte(Uint8 b, int lit)
{
	if(lit) {
		pushbyte(0x02, 0);
		pushbyte(0x01, 0);
	}
	p.data[p.ptr++] = b;
}

void
pushshort(Uint16 s, int lit)
{
	if(lit) {
		pushbyte(0x02, 0);
		pushbyte(0x02, 0);
	}
	pushbyte((s >> 8) & 0xff, 0);
	pushbyte(s & 0xff, 0);
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

#pragma mark - Parser

int
error(char *name, char *id)
{
	printf("Error: %s(%s)\n", name, id);
	return 0;
}

Uint8
findop(char *s)
{
	int i;
	for(i = 0; i < 32; ++i)
		if(scmp(opcodes[i], s))
			return i;
	return 0;
}

int
makelabel(char *id, Uint16 addr)
{
	Label *l;
	if(findlabel(id))
		return error("Label duplicate", id);
	l = &labels[labelslen++];
	scpy(id, l->name, 64);
	l->addr = addr;
	printf("New label: %s[0x%02x]\n", l->name, l->addr);
	return 1;
}

int
makeconst(char *id, FILE *f)
{
	char wv[64];
	fscanf(f, "%s", wv);
	return makelabel(id, shex(wv));
}

int
pass1(FILE *f)
{
	int skip = 0, vars = 0;
	Uint16 addr = 0;
	char w[64];
	while(fscanf(f, "%s", w) == 1) {
		if(iscomment(w, &skip)) continue;
		suca(w);
		if(w[0] == '@' && !makelabel(w + 1, addr))
			return error("Pass1 failed", w);
		if(w[0] == ';' && !makelabel(w + 1, vars++))
			return error("Pass1 failed", w);
		if(w[0] == ':') {
			if(!makeconst(w + 1, f))
				return error("Pass1 failed", w);
			else
				continue;
		}
		/* move addr ptr */
		if(findop(w) || scmp(w, "BRK"))
			addr += 1;
		else if(w[0] == '|')
			addr = shex(w + 1);
		else if(w[0] == '@')
			addr += 0;
		else if(w[0] == ';')
			addr += 0;
		else if(w[0] == '.')
			addr += 2;
		else if(w[0] == ',')
			addr += 4;
		else if(ismarker(w))
			addr += 0;
		else
			return error("Unknown label", w);
	}
	rewind(f);
	return 1;
}

int
pass2(FILE *f)
{
	int skip = 0;
	char w[64];
	while(fscanf(f, "%s", w) == 1) {
		Uint8 op = 0;
		Label *l;
		if(w[0] == '@') continue;
		if(w[0] == ';') continue;
		suca(w);
		if(iscomment(w, &skip) || ismarker(w)) continue;
		if(w[0] == '|')
			p.ptr = shex(w + 1);
		else if(w[0] == ':')
			fscanf(f, "%s", w);
		else if((op = findop(w)) || scmp(w, "BRK"))
			pushbyte(op, 0);
		else if((l = findlabel(w + 1)))
			pushshort(l->addr, w[0] == ',');
		else if(sihx(w + 1) && slen(w + 1) == 2)
			pushbyte(shex(w + 1), w[0] == ',');
		else if(sihx(w + 1) && slen(w + 1) == 4)
			pushshort(shex(w + 1), w[0] == ',');
		else
			return error("Unknown label", w);
	}
	return 1;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	if(argc < 3)
		return error("Input", "Missing");
	if(!(f = fopen(argv[1], "r")))
		return error("Open", "Failed");
	if(!pass1(f) || !pass2(f))
		return error("Assembly", "Failed");
	fwrite(p.data, sizeof(p.data), 1, fopen(argv[2], "wb"));
	fclose(f);
	printf("Assembled %s.\n", argv[2]);
	return 0;
}
