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
	Uint8 data[256 * 256];
	Uint16 ptr;
} Program;

typedef struct {
	Uint8 len, length[16], size, refs;
	char name[64], params[16][64];
} Macro;

typedef struct {
	Uint8 len, offset, refs;
	Uint16 addr;
	char name[64];
	Macro *macro;
} Label;

int macroslen;
Macro macros[256];

int labelslen;
Label labels[256];

Program p;

/* clang-format off */

char ops[][4] = {
	"BRK", "NOP", "LIT", "JMP", "JSR", "RTS", "LDR", "STR",
	"---", "---", "---", "---", "AND", "XOR", "ROL", "ROR",
	"POP", "DUP", "SWP", "OVR", "ROT", "---", "WSR", "RSW",
	"ADD", "SUB", "MUL", "DIV", "EQU", "NEQ", "GTH", "LTH"
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

void
pushtext(char *s, int lit)
{
	int i = 0;
	char c;
	if(lit)
		pushbyte(0x22, 0);
	while((c = s[i++]))
		pushbyte(c, 0);
	pushbyte(' ', 0);
}

Macro *
findmacro(char *s)
{
	int i;
	for(i = 0; i < macroslen; ++i)
		if(scmp(macros[i].name, s, 64))
			return &macros[i];
	return NULL;
}

Label *
findlabel(char *s)
{
	int i, rng = scin(s, '.');
	char name[64];
	scpy(s, name, rng > 0 ? rng + 1 : 64);
	for(i = 0; i < labelslen; ++i)
		if(scmp(labels[i].name, name, 64))
			return &labels[i];
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
	for(i = 0; i < l->macro->len; ++i) {
		if(scmp(l->macro->params[i], param, 64))
			return l->addr + o;
		o += l->macro->length[i];
	}
	printf("Warning %s.%s[%s]\n", l->name, param, l->macro->name);
	return 0;
}

Uint8
findlabellen(char *s)
{
	int i;
	char *param;
	Label *l = findlabel(s);
	if(scin(s, '.') < 1)
		return l->len;
	param = s + scin(s, '.') + 1;
	for(i = 0; i < l->macro->len; ++i)
		if(scmp(l->macro->params[i], param, 64))
			return l->macro->length[i];
	printf("Warning %s.%s[%s]\n", l->name, param, l->macro->name);
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
makemacro(char *name, FILE *f)
{
	Uint8 mode = 0;
	Macro *m;
	char wv[64];
	if(findmacro(name))
		return error("Macro duplicate", name);
	if(sihx(name))
		return error("Macro name is hex number", name);
	if(findopcode(name))
		return error("Macro name is invalid", name);
	m = &macros[macroslen++];
	scpy(name, m->name, 64);
	while(fscanf(f, "%s", wv)) {
		if(wv[0] == '{')
			continue;
		if(wv[0] == '}')
			break;
		if(mode == 0)
			scpy(wv, m->params[m->len], 64);
		else {
			m->length[m->len] = shex(wv);
			m->size += m->length[m->len];
			m->len++;
		}
		mode = !mode;
	}
	printf("New macro: %s[%d:%d]\n", name, m->len, m->size);
	return 1;
}

int
makelabel(char *name, Uint16 addr, Uint8 len, Macro *m)
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
	l->refs = 0;
	scpy(name, l->name, 64);
	if(m)
		l->macro = m;
	printf("New label: %s, at 0x%04x[%d]\n", l->name, l->addr, l->len);
	return 1;
}

int
makeconst(char *id, FILE *f)
{
	char wv[64];
	fscanf(f, "%s", wv);
	return makelabel(id, shex(wv), 1, 0);
}

int
makevariable(char *id, Uint16 *addr, FILE *f)
{
	char wv[64];
	Uint16 origin;
	Uint8 len;
	Macro *m = NULL;
	fscanf(f, "%s", wv);
	origin = *addr;
	if(sihx(wv))
		len = shex(wv);
	else if((m = findmacro(wv))) {
		len = m->size;
		m->refs++;
	} else
		return error("Invalid macro", wv);
	*addr += len;
	return makelabel(id, origin, len, m);
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
pass1(FILE *f)
{
	int ccmnt = 0, cbits = 0;
	Uint16 addr = 0;
	char w[64];
	printf("Pass 1\n");
	while(fscanf(f, "%s", w) == 1) {
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(skipblock(w, &cbits, '[', ']')) {
			if(w[0] == '[' || w[0] == ']')
				continue;
			if(sihx(w))
				addr += slen(w) == 4 ? 2 : 1;
			else
				addr += slen(w) + 1;
		} else if(w[0] == '@') {
			if(!makelabel(w + 1, addr, 0, NULL))
				return error("Pass1 failed", w);
		} else if(w[0] == ';') {
			if(!makevariable(w + 1, &addr, f))
				return error("Pass1 failed", w);
		} else if(w[0] == '&') {
			if(!makemacro(w + 1, f))
				return error("Pass1 failed", w);
		} else if(w[0] == ':') {
			if(!makeconst(w + 1, f))
				return error("Pass1 failed", w);
		} else if(findopcode(w) || scmp(w, "BRK", 4))
			addr += 1;
		else {
			switch(w[0]) {
			case '|':
				if(shex(w + 1) < addr)
					return error("Memory Overlap", w);
				addr = shex(w + 1);
				break;
			case '=': addr += 4; break; /* STR helper (lit addr-hb addr-lb str) */
			case '~': addr += 4; break; /* LDR helper (lit addr-hb addr-lb ldr) */
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
	int ccmnt = 0, cbits = 0, cmacro = 0;
	char w[64];
	printf("Pass 2\n");
	while(fscanf(f, "%s", w) == 1) {
		Uint8 op = 0;
		Label *l;
		if(w[0] == '@') continue;
		if(w[0] == '&') continue;
		if(skipblock(w, &ccmnt, '(', ')')) continue;
		if(skipblock(w, &cmacro, '{', '}')) continue;
		/* clang-format off */
		if(skipblock(w, &cbits, '[', ']')) {
			if(w[0] == '[' || w[0] == ']') { continue; }
			if(slen(w) == 4 && sihx(w)) pushshort(shex(w), 0); 
			else if(slen(w) == 2 && sihx(w)) pushbyte(shex(w), 0); 
			else pushtext(w, 0);
		}
		else if(w[0] == '|') p.ptr = shex(w + 1);
		else if((op = findopcode(w)) || scmp(w, "BRK", 4)) pushbyte(op, 0);
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
		else if(w[0] == '=' && (l = findlabel(w + 1)) && l->len){ pushshort(findlabeladdr(w+1), 1); pushbyte(findopcode(findlabellen(w+1) == 2 ? "STR2" : "STR"), 0); l->refs++;}
		else if(w[0] == '~' && (l = findlabel(w + 1)) && l->len){ pushshort(findlabeladdr(w+1), 1); pushbyte(findopcode(findlabellen(w+1) == 2 ? "LDR2" : "LDR"), 0); l->refs++;}
		else if(w[0] == '=' && sihx(w + 1)) { pushshort(shex(w + 1), 1); pushbyte(findopcode("STR2"), 0); }
		else if(w[0] == '~' && sihx(w + 1)) { pushshort(shex(w + 1), 1); pushbyte(findopcode("LDR2"), 0); }
		else if((l = findlabel(w + 1))) { pushshort(findlabeladdr(w+1), w[0] == ','); l->refs++; }
		else return error("Unknown label in second pass", w);
		/* clang-format on */
	}
	return 1;
}

void
cleanup(void)
{
	int i;
	for(i = 0; i < labelslen; ++i)
		if(!labels[i].refs)
			printf("--- Unused label: %s\n", labels[i].name);
	for(i = 0; i < macroslen; ++i)
		if(!macros[i].refs)
			printf("--- Unused macro: %s\n", macros[i].name);
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
	cleanup();
	return 0;
}
