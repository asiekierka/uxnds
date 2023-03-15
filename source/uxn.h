#include <nds.h>
#include <stdint.h>
#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#ifdef DEBUG
#define dprintf iprintf
#else
#define dprintf(...)
#endif

#ifdef BLOCKSDS
#define iprintf printf
#define siprintf sprintf
#define fiprintf fprintf
#endif

#define MAX_PATH 512
#define ITCM_ARM_CODE __attribute__((section(".itcm"), long_call, target("arm")))

typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint16_t Uint16;
typedef int16_t Sint16;
 typedef unsigned int Uint32;

#define PAGE_PROGRAM 0x0100
#define RAM_PAGES 0x0F

#define GETVEC(d) ((d)[0] << 8 | (d)[1])
#define POKDEV(x, y) { d[(x)] = (y) >> 8; d[(x) + 1] = (y); }
#define PEKDEV(o, x) { (o) = (d[(x)] << 8) + d[(x) + 1]; }

typedef struct {
	Uint8 ptr, kptr, error;
	Uint8 dat[256];
} Stack;

typedef struct {
	Uint16 ptr;
	Uint8 *dat;
} Memory;

typedef struct Uxn {
	Stack wst, rst;
	Memory ram;
	Uint8 dev[256];
	Uint8 (*dei)(struct Uxn *u, Uint8 addr);
	void (*deo)(struct Uxn *u, Uint8 addr, Uint8 value);
} Uxn;

struct Uxn;

typedef Uint8 Dei(Uxn *u, Uint8 addr);
typedef void Deo(Uxn *u, Uint8 addr, Uint8 value);

static inline void   poke8(Uint8 *m, Uint16 a, Uint8 b) { m[a] = b; }
static inline Uint8  peek8(Uint8 *m, Uint16 a) { return m[a]; }
static inline void   poke16(Uint8 *m, Uint16 a, Uint16 b) { poke8(m, a, b >> 8); poke8(m, a + 1, b); }
static inline Uint16 peek16(Uint8 *m, Uint16 a) { return (peek8(m, a) << 8) + peek8(m, a + 1); }

int uxn_boot(Uxn *u, Uint8 *ram, Dei *dei, Deo *deo);
int resetuxn(Uxn *c);
int uxn_eval(Uxn *u, Uint16 vec);
