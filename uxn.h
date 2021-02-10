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

#define FLAG_HALT 0x01
#define FLAG_SHORT 0x02
#define FLAG_SIGN 0x04
#define FLAG_COND 0x08

typedef struct {
	Uint8 ptr;
	Uint8 dat[256];
} Stack8;

typedef struct {
	Uint8 ptr;
	Uint16 dat[256];
} Stack16;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct Device {
	Uint8 len, mem[8];
	Uint8 (*rfn)(struct Device *, Uint8);
	Uint8 (*wfn)(struct Device *, Uint8);
} Device;

typedef struct {
	Uint8 literal, status, devices;
	Uint16 counter, vreset, vframe, verror;
	Stack8 wst;
	Stack16 rst;
	Memory ram;
	Device dev[256];
} Uxn;

void setflag(Uint8 *status, char flag, int b);
int getflag(Uint8 *status, char flag);
int loaduxn(Uxn *c, char *filepath);
int bootuxn(Uxn *c);
int evaluxn(Uxn *u, Uint16 vec);
Device *portuxn(Uxn *u, char *name, Uint8 (*onread)(Device *, Uint8), Uint8 (*onwrite)(Device *, Uint8));
