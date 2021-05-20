#include <stdio.h>
#include <stdlib.h>
#include "../../include/uxn.h"

/*
Copyright (u) 2021 Devine Lu Linvega
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#pragma mark - Operations

/* clang-format off */

static inline void   push8(Stack *s, Uint8 a) {
#ifdef CPU_ERROR_CHECKING
	if (s->ptr == 0xff) { s->error = 2; return; }
#endif
	s->dat[s->ptr++] = a;
}
static inline Uint8  pop8_keep(Stack *s) {
#ifdef CPU_ERROR_CHECKING
	if (s->kptr == 0) { s->error = 1; return 0; }
#endif
	return s->dat[--s->kptr];
}
static inline Uint8  pop8_nokeep(Stack *s) {
#ifdef CPU_ERROR_CHECKING
	if (s->ptr == 0) { s->error = 1; return 0; }
#endif
	return s->dat[--s->ptr];
}
static inline void   devpoke8(Device *d, Uint8 a, Uint8 b) { d->dat[a & 0xf] = b; d->talk(d, a & 0x0f, 1); }
static inline Uint8  devpeek8(Device *d, Uint8 a) { d->talk(d, a & 0x0f, 0); return d->dat[a & 0xf];  }
static inline void   push16(Stack *s, Uint16 a) { push8(s, a >> 8); push8(s, a); }
static inline void   devpoke16(Device *d, Uint8 a, Uint16 b) { devpoke8(d, a, b >> 8); devpoke8(d, a + 1, b); }
static inline Uint16 devpeek16(Device *d, Uint16 a) { return (devpeek8(d, a) << 8) + devpeek8(d, a + 1); }

/* clang-format on */

#pragma mark - Core

int
haltuxn(Uxn *u, char *name, int id)
{
	dprintf("Halted: %s#%04x, at 0x%04x\n", name, id, u->ram.ptr);
	u->ram.ptr = 0;
	return 0;
}

ITCM_ARM_CODE
static inline void
opcuxn(Uxn *u, Uint8 instr)
{
#ifdef CPU_ERROR_CHECKING
	// With CPU error checking enabled, the codebase becomes too large to fit in ITCM.
	// Therefore, we take some concessions.
	if (instr & 0x40) {
		u->src = &u->rst;
		u->dst = &u->wst;
	} else {
		u->src = &u->wst;
		u->dst = &u->rst;
	}

	switch (instr & 0xBF) {
#define UXN_SRC (u->src)
#define UXN_DST (u->dst)

#define UXN_KEEP_SYNC {}
#define pop8 pop8_nokeep
#define pop16(s) (pop8((s)) + (pop8((s)) << 8))

#define UXN_OPC(a) (a)
#include "uxn/opcodes.c"
#undef UXN_OPC

#undef pop16
#undef pop8
#undef UXN_KEEP_SYNC

#define UXN_KEEP_SYNC {(*(UXN_SRC)).kptr = (*(UXN_SRC)).ptr;}
#define pop8 pop8_keep
#define pop16(s) (pop8((s)) + (pop8((s)) << 8))

#define UXN_OPC(a) (a | 0x80)
#include "uxn/opcodes.c"
#undef UXN_OPC

#undef pop16
#undef pop8
#undef UXN_KEEP_SYNC

#undef UXN_DST
#undef UXN_SRC
	}
#else
	switch (instr) {
#define UXN_KEEP_SYNC {}
#define pop8 pop8_nokeep
#define pop16(s) (pop8((s)) + (pop8((s)) << 8))

#define UXN_OPC(a) (a)
#define UXN_SRC (&u->wst)
#define UXN_DST (&u->rst)
#include "uxn/opcodes.c"
#undef UXN_DST
#undef UXN_SRC
#undef UXN_OPC

#define UXN_OPC(a) (a | 0x40)
#define UXN_SRC (&u->rst)
#define UXN_DST (&u->wst)
#include "uxn/opcodes.c"
#undef UXN_DST
#undef UXN_SRC
#undef UXN_OPC

#undef pop16
#undef pop8
#undef UXN_KEEP_SYNC

#define UXN_KEEP_SYNC {(*(UXN_SRC)).kptr = (*(UXN_SRC)).ptr;}
#define pop8 pop8_keep
#define pop16(s) (pop8((s)) + (pop8((s)) << 8))

#define UXN_OPC(a) (a | 0x80)
#define UXN_SRC (&u->wst)
#define UXN_DST (&u->rst)
#include "uxn/opcodes.c"
#undef UXN_DST
#undef UXN_SRC
#undef UXN_OPC

#define UXN_OPC(a) (a | 0xC0)
#define UXN_SRC (&u->rst)
#define UXN_DST (&u->wst)
#include "uxn/opcodes.c"
#undef UXN_DST
#undef UXN_SRC
#undef UXN_OPC

#undef pop16
#undef pop8
#undef UXN_KEEP_SYNC
	}
#endif
}

ITCM_ARM_CODE
static inline int
stepuxn(Uxn *u, Uint8 instr)
{
	opcuxn(u, instr);
#ifdef CPU_ERROR_CHECKING
	if(u->wst.error)
		return haltuxn(u, u->wst.error == 1 ? "Working-stack underflow" : "Working-stack overflow", instr);
	if(u->rst.error)
		return haltuxn(u, u->rst.error == 1 ? "Return-stack underflow" : "Return-stack overflow", instr);
#endif
	return 1;
}

ITCM_ARM_CODE
int
evaluxn(Uxn *u, Uint16 vec)
{
	u->ram.ptr = vec;
#ifdef CPU_ERROR_CHECKING
	u->wst.error = 0;
	u->rst.error = 0;
#endif
	while(u->ram.ptr)
		if(!stepuxn(u, u->ram.dat[u->ram.ptr++]))
			return 0;
	return 1;
}

int
bootuxn(Uxn *u)
{
	memset(u, 0, sizeof(*u));
	u->ram.dat = malloc(65536);
	memset(u->ram.dat, 0, 65536);
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb")))
		return haltuxn(u, "Missing input rom.", 0);
	fread(u->ram.dat + PAGE_PROGRAM, sizeof(u->ram.dat) - PAGE_PROGRAM, 1, f);
	dprintf("Uxn loaded[%s].\n", filepath);
	return 1;
}

Device *
portuxn(Uxn *u, Uint8 id, char *name, void (*talkfn)(Device *d, Uint8 b0, Uint8 w))
{
	Device *d = &u->dev[id];
	d->addr = id * 0x10;
	d->u = u;
	d->mem = u->ram.dat;
	d->talk = talkfn;
	dprintf("Device added #%02x: %s, at 0x%04x \n", id, name, d->addr);
	return d;
}
