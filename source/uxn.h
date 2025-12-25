#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define dbgprintf iprintf
#else
#define dbgprintf(...)
#endif

#ifdef __BLOCKSDS__
#define iprintf printf
#define siprintf sprintf
#define sniprintf snprintf
#define fiprintf fprintf
#endif

#define MAX_PATH 1024
#ifdef __NDS__
#define ITCM_ARM_CODE __attribute__((section(".itcm"), long_call, target("arm")))
#else
#define ITCM_ARM_CODE
#endif

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
#define POKE2(d, v) { (d)[0] = (v) >> 8; (d)[1] = (v); }
#define PEEK2(d) ((d)[0] << 8 | (d)[1])

static inline void   poke8(Uint8 *m, Uint16 a, Uint8 b) { m[a] = b; }
static inline Uint8  peek8(Uint8 *m, Uint16 a) { return m[a]; }
static inline void   poke16(Uint8 *m, Uint16 a, Uint16 b) { poke8(m, a, b >> 8); poke8(m, a + 1, b); }
static inline Uint16 peek16(Uint8 *m, Uint16 a) { return (peek8(m, a) << 8) + peek8(m, a + 1); }

typedef Uint8 (*uxn_dei_t)(Uint8*, Uint8);
typedef void (*uxn_deo_t)(Uint8*, Uint8);

int uxn_get_wst_ptr(void);
int uxn_get_rst_ptr(void);
void uxn_set_wst_ptr(int value);
void uxn_set_rst_ptr(int value);
void uxn_register_device(int id, uxn_dei_t dei, uxn_deo_t deo);

int resetuxn(void);
int uxn_boot(void);

// Legacy API

typedef struct {
	Uint8 *dat;
} Stack;

typedef struct {
	Uint8 *dat;
} Memory;

typedef struct Uxn {
	Stack wst, rst;
	Memory ram;
	Uint8 *dev;
} Uxn;

struct Uxn;

extern Uxn u;

int uxn_eval(Uxn *u, Uint32 vec);
