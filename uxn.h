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

#define FLAG_HALT 0x01
#define FLAG_SHORT 0x02
#define FLAG_SIGN 0x04
#define FLAG_COND 0x08

typedef struct {
	Uint8 ptr;
	Uint8 dat[256];
} St8;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct Device {
	Uint8 len, mem[8];
	Uint8 (*read)(struct Device *, Memory *, Uint8);
	Uint8 (*write)(struct Device *, Memory *, Uint8);
} Device;

typedef struct {
	Uint8 literal, status, balance, devices;
	Uint16 counter, devr, devw, vreset, vframe, verror;
	St8 wst, rst;
	Memory ram;
	Device dev[256];
} Uxn;

void setflag(Uint8 *status, char flag, int b);
int getflag(Uint8 *status, char flag);
int loaduxn(Uxn *c, char *filepath);
int bootuxn(Uxn *c);
int evaluxn(Uxn *u, Uint16 vec);
Device *portuxn(Uxn *u, char *name, Uint8 (*rfn)(Device *, Memory *, Uint8), Uint8 (*wfn)(Device *, Memory *, Uint8));
