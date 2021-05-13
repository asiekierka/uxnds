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

#define PAGE_PROGRAM 0x0100

typedef struct {
	Uint8 ptr, kptr, error;
	Uint8 dat[256];
} Stack;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct Device {
	struct Uxn *u;
	Uint8 addr, dat[16], *mem;
	void (*talk)(struct Device *d, Uint8, Uint8);
} Device;

typedef struct Uxn {
	Stack wst, rst, *src, *dst;
	Memory ram;
	Device dev[16];
} Uxn;

struct Uxn;

void mempoke16(Uint8 *m, Uint16 a, Uint16 b);
Uint16 mempeek16(Uint8 *m, Uint16 a);

int loaduxn(Uxn *c, char *filepath);
int bootuxn(Uxn *c);
int evaluxn(Uxn *u, Uint16 vec);
Device *portuxn(Uxn *u, Uint8 id, char *name, void (*talkfn)(Device *, Uint8, Uint8));
