#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define TRIM 0x0100

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;

typedef struct {
	char name[64], items[64][64];
	Uint8 len, refs;
} Macro;

typedef struct {
	char name[64];
	Uint8 refs;
	Uint16 addr;
} Label;

typedef struct {
	Uint8 data[256 * 256], llen, mlen;
	Uint16 ptr, length;
	Label labels[256];
	Macro macros[256];
} Program;

Program p;

/* clang-format off */

char ops[][4] = {
	"BRK", "LIT", "NOP", "POP", "DUP", "SWP", "OVR", "ROT",
	"EQU", "NEQ", "GTH", "LTH", "GTS", "LTS", "IOR", "IOW",
	"PEK", "POK", "LDR", "STR", "JMP", "JNZ", "JSR", "STH",
	"ADD", "SUB", "MUL", "DIV", "AND", "ORA", "EOR", "SFT"
};

int   scin(char *s, char c) { int i = 0; while(s[i]) if(s[i++] == c) return i - 1; return -1; } /* string char index */
int   scmp(char *a, char *b, int len) { int i = 0; while(a[i] == b[i] && i < len) if(!a[i++]) return 1; return 0; } /* string compare */
int   slen(char *s) { int i = 0; while(s[i] && s[++i]) ; return i; } /* string length */
int   sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) return 0; return 1; } /* string is hexadecimal */
int   shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */

#pragma mark - Helpers

/* clang-format on */

#pragma mark - I/O

void
pushbyte(Uint8 b, int lit)
{
	if(lit) pushbyte(0x01, 0);
	p.data[p.ptr++] = b;
	p.length = p.ptr;
}

void
pushshort(Uint16 s, int lit)
{
	if(lit) pushbyte(0x21, 0);
	pushbyte((s >> 8) & 0xff, 0);
	pushbyte(s & 0xff, 0);
}

void
pushtext(char *s, int lit)
{
	int i = 0;
	char c;
	if(lit) pushbyte(0x21, 0);
	while((c = s[i++])) pushbyte(c, 0);
}

Macro *
findmacro(char *name)
{
	int i;
	for(i = 0; i < p.mlen; ++i)
		if(scmp(p.macros[i].name, name, 64))
			return &p.macros[i];
	return NULL;
}

Label *
findlabel(char *name)
{
	int i;
	for(i = 0; i < p.llen; ++i)
		if(scmp(p.labels[i].name, name, 64))
			return &p.labels[i];
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
			if(s[3 + m] == '2')
				i |= (1 << 5); /* mode: short */
			else if(s[3 + m] == 'r')
				i |= (1 << 6); /* mode: return */
			else
				return 0; /* failed to match */
			m++;
		}
		return i;
	}
	return 0;
}

char *
sublabel(char *src, char *scope, char *name)
{
	scpy(scope, src, 64);
	scpy("/", src + slen(src), 64);
	scpy(name, src + slen(src), 64);
	return src;
}

#pragma mark - Parser

int
error(char *name, char *id)
{
	printf("Error: %s[%s]\n", name, id);
	return 0;
}

int
makemacro(char *name, FILE *f)
{
	Macro *m;
	char word[64];
	if(findmacro(name))
		return error("Macro duplicate", name);
	if(sihx(name) && slen(name) % 2 == 0)
		return error("Macro name is hex number", name);
	if(findopcode(name))
		return error("Macro name is invalid", name);
	m = &p.macros[p.mlen++];
	scpy(name, m->name, 64);
	while(fscanf(f, "%s", word)) {
		if(word[0] == '{') continue;
		if(word[0] == '}') break;
		if(m->len > 64)
			return error("Macro too large", name);
		if(slen(word) >= 64)
			return error("Word too long", name);
		scpy(word, m->items[m->len++], 64);
	}
	printf("New macro: %s, %d items\n", m->name, m->len);
	return 1;
}

int
makelabel(char *name, Uint16 addr)
{
	Label *l;
	if(findlabel(name))
		return error("Label duplicate", name);
	if(sihx(name) && slen(name) % 2 == 0)
		return error("Label name is hex number", name);
	if(findopcode(name))
		return error("Label name is invalid", name);
	l = &p.labels[p.llen++];
	l->addr = addr;
	l->refs = 0;
	scpy(name, l->name, 64);
	printf("New label: %s, at 0x%04x\n", l->name, l->addr);
	return 1;
}

int
skipblock(char *w, int *cap, char a, char b)
{
	if(w[0] == b) {
		*cap = 0;
		return 1;
	}
	if(w[0] == a) *cap = 1;
	if(*cap) return 1;
	return 0;
}

int
walktoken(char *w)
{
	Macro *m;
	if(findopcode(w) || scmp(w, "BRK", 4))
		return 1;
	switch(w[0]) {
	case '[': return 0;
	case ']': return 0;
	case '.': return 2; /* zero-page: LIT addr-lb */
	case ',': return 2; /* relative:  LIT addr-rel */
	case ':': return 2; /* absolute:      addr-hb addr-lb */
	case ';': return 3; /* absolute:  LIT addr-hb addr-lb */
	case '$': return shex(w + 1);
	case '#': return slen(w + 1) == 4 ? 3 : 2;
	}
	if((m = findmacro(w))) {
		int i, res = 0;
		for(i = 0; i < m->len; ++i)
			res += walktoken(m->items[i]);
		return res;
	}
	return error("Unknown label in first pass", w);
}

int
parsetoken(char *w)
{
	Label *l;
	Macro *m;
	if(w[0] == '.' && (l = findlabel(w + 1))) { /* zero-page */
		pushbyte(l->addr, 1);
		return ++l->refs;
	} else if(w[0] == ',' && (l = findlabel(w + 1))) {
		int off = l->addr - p.ptr - 3;
		if(off < -126 || off > 126)
			return error("Address is too far", w);
		pushbyte((Sint8)off, 1);
		return ++l->refs;
	} else if(w[0] == ':' && (l = findlabel(w + 1))) { /* absolute */
		pushshort(l->addr, 0);
		return ++l->refs;
	} else if(w[0] == ';' && (l = findlabel(w + 1))) { /* absolute */
		pushshort(l->addr, 1);
		return ++l->refs;
	} else if(findopcode(w) || scmp(w, "BRK", 4)) {
		pushbyte(findopcode(w), 0);
		return 1;
	} else if(w[0] == '#') {
		if(slen(w + 1) == 1)
			pushbyte((Uint8)w[1], 1);
		if(sihx(w + 1) && slen(w + 1) == 2)
			pushbyte(shex(w + 1), 1);
		else if(sihx(w + 1) && slen(w + 1) == 4)
			pushshort(shex(w + 1), 1);
		else
			return 0;
		return 1;
	} else if((m = findmacro(w))) {
		int i;
		m->refs++;
		for(i = 0; i < m->len; ++i)
			if(!parsetoken(m->items[i]))
				return 0;
		return 1;
	} else if(sihx(w)) {
		if(slen(w) == 2)
			pushbyte(shex(w), 0);
		else if(slen(w) == 4)
			pushshort(shex(w), 0);
		return 1;
	}
	return 0;
}

int
pass1(FILE *f)
{
	int ccmnt = 0;
	Uint16 addr = 0;
	char w[64], scope[64], subw[64];
	printf("Pass 1\n");
	while(fscanf(f, "%s", w) == 1) {
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(w[0] == '|') {
			addr = shex(w + 1);
		} else if(w[0] == '%') {
			if(!makemacro(w + 1, f))
				return error("Pass1 failed", w);
		} else if(w[0] == '@') {
			if(!makelabel(w + 1, addr))
				return error("Pass1 failed", w);
			scpy(w + 1, scope, 64);
		} else if(w[0] == '&') {
			if(!makelabel(sublabel(subw, scope, w + 1), addr))
				return error("Pass1 failed", w);
		} else if(sihx(w))
			addr += slen(w) / 2;
		else
			addr += walktoken(w);
	}
	rewind(f);
	return 1;
}

int
pass2(FILE *f)
{
	int ccmnt = 0, ctemplate = 0;
	char w[64], scope[64], subw[64];
	printf("Pass 2\n");
	while(fscanf(f, "%s", w) == 1) {
		if(w[0] == '%') continue;
		if(w[0] == '&') continue;
		if(w[0] == '[') continue;
		if(w[0] == ']') continue;
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(skipblock(w, &ctemplate, '{', '}')) continue;
		if(w[0] == '|') {
			if(p.length && shex(w + 1) < p.ptr)
				return error("Memory Overwrite", w);
			p.ptr = shex(w + 1);
			continue;
		} else if(w[0] == '$') {
			p.ptr += shex(w + 1);
			continue;
		} else if(w[0] == '@') {
			scpy(w + 1, scope, 64);
			continue;
		}
		if(w[1] == '&')
			scpy(sublabel(subw, scope, w + 2), w + 1, 64);
		if(!parsetoken(w))
			return error("Unknown label in second pass", w);
	}
	return 1;
}

void
cleanup(char *filename)
{
	int i;
	printf("Assembled %s(%d bytes), %d labels, %d macros.\n\n", filename, (p.length - TRIM), p.llen, p.mlen);
	for(i = 0; i < p.llen; ++i)
		if(!p.labels[i].refs)
			printf("--- Unused label: %s\n", p.labels[i].name);
	for(i = 0; i < p.mlen; ++i)
		if(!p.macros[i].refs)
			printf("--- Unused macro: %s\n", p.macros[i].name);
}

int
main(int argc, char *argv[])
{
	FILE *f;
	if(argc < 3)
		return !error("Input", "Missing");
	if(!(f = fopen(argv[1], "r")))
		return !error("Open", "Failed");
	if(!pass1(f) || !pass2(f))
		return !error("Assembly", "Failed");
	fwrite(p.data + TRIM, p.length - TRIM, 1, fopen(argv[2], "wb"));
	fclose(f);
	cleanup(argv[2]);
	return 0;
}
