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
	char name[64], items[16][64];
	Uint8 len, refs;
} Macro;

typedef struct {
	char name[64];
	unsigned int size;
} Map;

typedef struct {
	char name[64];
	Uint8 refs, maps;
	Uint16 addr;
	Map map[16];
} Label;

typedef struct {
	Uint8 data[256 * 256], llen, mlen;
	Uint16 ptr, count;
	Label labels[256];
	Macro macros[256];
} Program;

Program p;

/* clang-format off */

char ops[][4] = {
	"BRK", "NOP", "LIT", "LDR", "STR", "---", "JMP", "JSR", 
	"EQU", "NEQ", "GTH", "LTH", "AND", "ORA", "EOR", "SFT",
	"POP", "DUP", "SWP", "OVR", "ROT", "---", "CLN", "STH",
	"ADD", "SUB", "MUL", "DIV", "---", "---", "---", "---"
};

int   scin(char *s, char c) { int i = 0; while(s[i]) if(s[i++] == c) return i - 1; return -1; } /* string char index */
int   scmp(char *a, char *b, int len) { int i = 0; while(a[i] == b[i] && i < len) if(!a[i++]) return 1; return 0; } /* string compare */
int   slen(char *s) { int i = 0; while(s[i] && s[++i]) ; return i; } /* string length */
int   sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) return 0; return 1; } /* string is hexadecimal */
int   shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'A' && c <= 'F') n = n * 16 + 10 + (c - 'A'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */

#pragma mark - Helpers

/* clang-format on */

#pragma mark - I/O

void
pushbyte(Uint8 b, int lit)
{
	if(lit) pushbyte(0x02, 0);
	p.data[p.ptr++] = b;
	p.count++;
}

void
pushshort(Uint16 s, int lit)
{
	if(lit) pushbyte(0x22, 0);
	pushbyte((s >> 8) & 0xff, 0);
	pushbyte(s & 0xff, 0);
}

void
pushtext(char *s, int lit)
{
	int i = 0;
	char c;
	if(lit) pushbyte(0x22, 0);
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
findlabel(char *s)
{
	int i, rng = scin(s, '.');
	char name[64];
	scpy(s, name, rng > 0 ? rng + 1 : 64);
	for(i = 0; i < p.llen; ++i)
		if(scmp(p.labels[i].name, name, 64))
			return &p.labels[i];
	return NULL;
}

Uint16
findlabeladdr(char *s)
{
	int i, o = 0;
	char *param;
	Label *l = findlabel(s);
	if(scin(s, '.') < 1)
		return l->addr;
	param = s + scin(s, '.') + 1;
	for(i = 0; i < l->maps; ++i) {
		if(scmp(l->map[i].name, param, 64))
			return l->addr + o;
		o += l->map[i].size;
	}
	printf("!!! Warning %s.%s\n", l->name, param);
	return 0;
}

Uint8
findlabellen(char *s)
{
	int i;
	char *param;
	Label *l = findlabel(s);
	if(scin(s, '.') < 1)
		return l->map[0].size;
	param = s + scin(s, '.') + 1;
	for(i = 0; i < l->maps; ++i)
		if(scmp(l->map[i].name, param, 64))
			return l->map[i].size;
	printf("!!! Warning %s.%s\n", l->name, param);
	return 0;
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
			if(s[3 + m] == 'r') i |= (1 << 6); /* mode: return */
			if(s[3 + m] == '?') i |= (1 << 7); /* mode: conditional */
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
	scpy("-", src + slen(src), 64);
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
		scpy(word, m->items[m->len++], 64);
	}
	printf("New macro: %s(%d items)\n", m->name, m->len);
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
makevariable(char *name, Uint16 *addr, FILE *f)
{
	Label *l;
	char word[64];
	if(!makelabel(name, *addr))
		return error("Could not create variable", name);
	l = findlabel(name);
	while(fscanf(f, "%s", word)) {
		if(word[0] == '{') continue;
		if(word[0] == '}') break;
		scpy(word, l->map[l->maps].name, 64);
		fscanf(f, "%u", &l->map[l->maps].size);
		*addr += l->map[l->maps++].size;
	}
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
	case '=': return 4; /* STR helper (lit addr-hb addr-lb str) */
	case '~': return 4; /* LDR helper (lit addr-hb addr-lb ldr) */
	case ',': return 3; /* lit2 addr-hb addr-lb */
	case '.': return 2; /* addr-hb addr-lb */
	case '^': return 2; /* Relative jump: lit addr-offset */
	case '#': return (slen(w + 1) == 2 ? 2 : 3);
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
	Uint8 op = 0;
	Label *l;
	Macro *m;

	if(w[0] == '^' && (l = findlabel(w + 1))) {
		int off = l->addr - p.ptr - 3;
		if(off < -126 || off > 126) {
			printf("Address %s is too far(%d).\n", w, off);
			return 0;
		}
		pushbyte((Sint8)(l->addr - p.ptr - 3), 1);
		l->refs++;
		return 1;
	} else if(w[0] == '=' && (l = findlabel(w + 1))) {
		if(!findlabellen(w + 1) || findlabellen(w + 1) > 2)
			return error("Invalid store helper", w);
		pushshort(findlabeladdr(w + 1), 1);
		pushbyte(findopcode(findlabellen(w + 1) == 2 ? "STR2" : "STR"), 0);
		l->refs++;
		return 1;
	} else if(w[0] == '~' && (l = findlabel(w + 1))) {
		if(!findlabellen(w + 1) || findlabellen(w + 1) > 2)
			return error("Invalid load helper", w);
		pushshort(findlabeladdr(w + 1), 1);
		pushbyte(findopcode(findlabellen(w + 1) == 2 ? "LDR2" : "LDR"), 0);
		l->refs++;
		return 1;
	} else if((op = findopcode(w)) || scmp(w, "BRK", 4)) {
		pushbyte(op, 0);
		return 1;
	} else if(w[0] == '.' && (l = findlabel(w + 1))) {
		pushshort(findlabeladdr(w + 1), 0);
		l->refs++;
		return 1;
	} else if(w[0] == ',' && (l = findlabel(w + 1))) {
		pushshort(findlabeladdr(w + 1), 1);
		l->refs++;
		return 1;
	} else if(w[0] == '#' && sihx(w + 1)) {
		if(slen(w + 1) == 2)
			pushbyte(shex(w + 1), 1);
		else if(slen(w + 1) == 4)
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
	}
	return 0;
}

int
pass1(FILE *f)
{
	int ccmnt = 0, cbits = 0;
	Uint16 addr = 0;
	char w[64], scope[64], subw[64];
	printf("Pass 1\n");
	while(fscanf(f, "%s", w) == 1) {
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(skipblock(w, &cbits, '[', ']')) {
			if(w[0] == '[' || w[0] == ']')
				continue;
			if(sihx(w))
				addr += slen(w) == 4 ? 2 : 1;
			else
				addr += slen(w);
		} else if(w[0] == '%') {
			if(!makemacro(w + 1, f))
				return error("Pass1 failed", w);
			scpy(w + 1, scope, 64);
		} else if(w[0] == '@') {
			if(!makelabel(w + 1, addr))
				return error("Pass1 failed", w);
			scpy(w + 1, scope, 64);
		} else if(w[0] == '$') {
			if(!makelabel(sublabel(subw, scope, w + 1), addr))
				return error("Pass1 failed", w);
		} else if(w[0] == ';') {
			if(!makevariable(w + 1, &addr, f))
				return error("Pass1 failed", w);
		} else if(w[0] == '|') {
			if(shex(w + 1) < addr)
				return error("Memory Overwrite", w);
			addr = shex(w + 1);
		} else
			addr += walktoken(w);
	}
	rewind(f);
	return 1;
}

int
pass2(FILE *f)
{
	int ccmnt = 0, cbits = 0, ctemplate = 0;
	char w[64], scope[64], subw[64];
	printf("Pass 2\n");
	while(fscanf(f, "%s", w) == 1) {
		if(w[0] == ';') continue;
		if(w[0] == '$') continue;
		if(w[0] == '%') continue;
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(skipblock(w, &ctemplate, '{', '}')) continue;
		if(w[0] == '|') {
			p.ptr = shex(w + 1);
			continue;
		}
		if(w[0] == '@') {
			scpy(w + 1, scope, 64);
			continue;
		}
		if(w[1] == '$') {
			scpy(sublabel(subw, scope, w + 2), w + 1, 64);
		}
		if(skipblock(w, &cbits, '[', ']')) {
			if(w[0] == '[' || w[0] == ']') { continue; }
			if(slen(w) == 4 && sihx(w))
				pushshort(shex(w), 0);
			else if(slen(w) == 2 && sihx(w))
				pushbyte(shex(w), 0);
			else
				pushtext(w, 0);
		} else if(!parsetoken(w)) {
			return error("Unknown label in second pass", w);
		}
	}
	return 1;
}

void
cleanup(char *filename)
{
	int i;
	printf("Assembled %s(%0.2fkb), %d labels, %d macros.\n\n", filename, p.count / 1000.0, p.llen, p.mlen);
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
	if(argc < 3) {
		error("Input", "Missing");
		return 1;
	}
	if(!(f = fopen(argv[1], "r"))) {
		error("Open", "Failed");
		return 1;
	}
	if(!pass1(f) || !pass2(f)) {
		error("Assembly", "Failed");
		return 1;
	}
	fwrite(p.data, sizeof(p.data), 1, fopen(argv[2], "wb"));
	fclose(f);
	cleanup(argv[2]);
	return 0;
}
