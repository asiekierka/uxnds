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
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;

typedef struct {
	int ptr;
	Uint8 data[65536];
} Program;

typedef struct {
	Uint8 len;
	Uint16 addr;
	char name[64];
} Label;

int labelslen;
Label labels[256];
Program p;

/* clang-format off */

char ops[][4] = {
	"BRK", "NOP", "LIT", "---", "IOR", "IOW", "LDR", "STR",
	"JMP", "JSR", "RTI", "RTS", "---", "---", "---", "---",
	"POP", "DUP", "SWP", "OVR", "ROT", "AND", "ORA", "ROL",
	"ADD", "SUB", "MUL", "DIV", "EQU", "NEQ", "GTH", "LTH"
};

int scmp(char *a, char *b) { int i = 0; while(a[i] == b[i]) if(!a[i++]) return 1; return 0; } /* string compare */
int slen(char *s) { int i = 0; while(s[i] && s[++i]) ; return i; } /* string length */
int sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) return 0; return 1; } /* string is hexadecimal */
int shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'A' && c <= 'F') n = n * 16 + 10 + (c - 'A'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */

#pragma mark - Helpers

/* clang-format on */

#pragma mark - I/O

void
pushbyte(Uint8 b, int lit)
{
	if(lit)
		pushbyte(0x02, 0);
	p.data[p.ptr++] = b;
}

void
pushshort(Uint16 s, int lit)
{
	if(lit)
		pushbyte(0x22, 0);
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

Uint8
findopcode(char *s)
{
	int i;
	for(i = 0; i < 0x20; ++i) {
		int m = 0;
		char *o = ops[i];
		if(o[0] != s[0] || o[1] != s[1] || o[2] != s[2])
			continue;
		while(s[3 + m]) {
			if(s[3 + m] == '2') i |= (1 << 5); /* mode: short */
			if(s[3 + m] == 'S') i |= (1 << 6); /* mode: signed */
			if(s[3 + m] == '?') i |= (1 << 7); /* mode: conditional */
			m++;
		}
		return i;
	}
	return 0;
}

#pragma mark - Parser

int
error(char *name, char *id)
{
	printf("Error: %s[%s]\n", name, id);
	return 0;
}

int
makelabel(char *name, Uint16 addr, Uint8 len)
{
	Label *l;
	if(findlabel(name))
		return error("Label duplicate", name);
	if(sihx(name))
		return error("Label name is hex number", name);
	if(findopcode(name))
		return error("Label name is invalid", name);
	l = &labels[labelslen++];
	l->addr = addr;
	l->len = len;
	scpy(name, l->name, 64);
	printf("New label: %s, at 0x%02x[%d]\n", l->name, l->addr, l->len);
	return 1;
}

int
makeconst(char *id, FILE *f)
{
	char wv[64];
	fscanf(f, "%s", wv);
	return makelabel(id, shex(wv), 1);
}

int
makevariable(char *id, Uint16 *addr, FILE *f)
{
	char wv[64];
	Uint8 origin;
	fscanf(f, "%s", wv);
	origin = *addr;
	*addr += shex(wv);
	return makelabel(id, origin, shex(wv));
}

int
skipcomment(char *w, int *cap)
{
	if(w[0] == ')') {
		*cap = 0;
		return 1;
	}
	if(w[0] == '(') *cap = 1;
	if(*cap) return 1;
	return 0;
}

int
skipstring(char *w, int *cap, Uint16 *addr)
{
	if(w[0] == '"') {
		if(*cap)
			*addr += 1;
		*cap = !(*cap);
		return 1;
	}
	if(*cap) {
		*addr += slen(w) + 1;
		return 1;
	}
	return 0;
}

int
capturestring(char *w, int *cap)
{
	if(w[0] == '"') {
		if(*cap)
			pushbyte(0x00, 0);
		*cap = !(*cap);
		return 1;
	}
	if(*cap) {
		int i;
		for(i = 0; i < slen(w); ++i)
			pushbyte(w[i], 0);
		pushbyte(' ', 0);
		return 1;
	}
	return 0;
}

int
pass1(FILE *f)
{
	int ccmnt = 0, cstrg = 0;
	Uint16 addr = 0;
	char w[64];
	while(fscanf(f, "%s", w) == 1) {
		if(skipcomment(w, &ccmnt)) continue;
		if(skipstring(w, &cstrg, &addr)) continue;
		if(w[0] == '@') {
			if(!makelabel(w + 1, addr, 0))
				return error("Pass1 failed", w);
		} else if(w[0] == ';') {
			if(!makevariable(w + 1, &addr, f))
				return error("Pass1 failed", w);
		} else if(w[0] == ':') {
			if(!makeconst(w + 1, f))
				return error("Pass1 failed", w);
		} else if(findopcode(w) || scmp(w, "BRK"))
			addr += 1;
		else {
			switch(w[0]) {
			case '|': addr = shex(w + 1); break;
			case '=': addr += 4; break; /* STR helper */
			case '~': addr += 4; break; /* LDR helper */
			case ',': addr += 3; break;
			case '.': addr += (slen(w + 1) == 2 ? 1 : 2); break;
			case '+': /* signed positive */
			case '-': /* signed negative */
			case '#': addr += (slen(w + 1) == 2 ? 2 : 3); break;
			default: return error("Unknown label in first pass", w);
			}
		}
	}
	rewind(f);
	return 1;
}

int
pass2(FILE *f)
{
	int ccmnt = 0, cstrg = 0;
	char w[64];
	while(fscanf(f, "%s", w) == 1) {
		Uint8 op = 0;
		Label *l;
		if(w[0] == '@') continue;
		if(skipcomment(w, &ccmnt)) continue;
		if(capturestring(w, &cstrg)) continue;
		/* clang-format off */
		if(w[0] == '|') p.ptr = shex(w + 1);
		else if((op = findopcode(w)) || scmp(w, "BRK")) pushbyte(op, 0);
		else if(w[0] == ':') fscanf(f, "%s", w);
		else if(w[0] == ';') fscanf(f, "%s", w);
		else if(w[0] == '.' && sihx(w + 1) && slen(w + 1) == 2) pushbyte(shex(w + 1), 0);
		else if(w[0] == '.' && sihx(w + 1) && slen(w + 1) == 4) pushshort(shex(w + 1), 0);
		else if(w[0] == '#' && sihx(w + 1) && slen(w + 1) == 2) pushbyte(shex(w + 1), 1); 
		else if(w[0] == '#' && sihx(w + 1) && slen(w + 1) == 4) pushshort(shex(w + 1), 1);
		else if(w[0] == '+' && sihx(w + 1) && slen(w + 1) == 2) pushbyte((Sint8)shex(w + 1), 1);
		else if(w[0] == '+' && sihx(w + 1) && slen(w + 1) == 4) pushshort((Sint16)shex(w + 1), 1);
		else if(w[0] == '-' && sihx(w + 1) && slen(w + 1) == 2) pushbyte((Sint8)(shex(w + 1) * -1), 1);
		else if(w[0] == '-' && sihx(w + 1) && slen(w + 1) == 4) pushshort((Sint16)(shex(w + 1) * -1), 1);
		else if(w[0] == '=' && (l = findlabel(w + 1)) && l->len){ pushshort(l->addr, 1); pushbyte(findopcode(l->len == 2 ? "STR2" : "STR"),0); }
		else if(w[0] == '~' && (l = findlabel(w + 1)) && l->len){ pushshort(l->addr, 1); pushbyte(findopcode(l->len == 2 ? "LDR2" : "LDR"),0); }
		else if((l = findlabel(w + 1))) pushshort(l->addr, w[0] == ',');
		else return error("Unknown label in second pass", w);
		/* clang-format on */
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
	printf("Assembled %s.\n\n", argv[2]);
	return 0;
}
