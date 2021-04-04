#include <stdio.h>

/*
Copyright (u) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"

#pragma mark - Operations

/* clang-format off */
void   setflag(Uint8 *a, char flag, int b) { if(b) *a |= flag; else *a &= (~flag); }
int    getflag(Uint8 *a, char flag) { return *a & flag; }
Uint8  devpoke8(Uxn *u, Uint8 id, Uint8 b0, Uint8 b1){ return id < 0x10 ? u->dev[id].poke(u, PAGE_DEVICE + id * 0x10, b0, b1) : b1; }

void   push8(Stack *s, Uint8 a) { if (s->ptr == 0xff) { s->error = 2; return; } s->dat[s->ptr++] = a; }
Uint8  pop8(Stack *s) { if (s->ptr == 0) { s->error = 1; return 0; } return s->dat[--s->ptr]; }
Uint8  peek8(Stack *s, Uint8 a) { if (s->ptr < a + 1) s->error = 1; return s->dat[s->ptr - a - 1]; }
void   mempoke8(Uxn *u, Uint16 a, Uint8 b) { u->ram.dat[a] = (a & 0xff00) == PAGE_DEVICE ? devpoke8(u, (a & 0xff) >> 4, a & 0xf, b) : b; }
Uint8  mempeek8(Uxn *u, Uint16 a) { return u->ram.dat[a]; }

void   push16(Stack *s, Uint16 a) { push8(s, a >> 8); push8(s, a); }
Uint16 pop16(Stack *s) { return pop8(s) + (pop8(s) << 8); }
Uint16 peek16(Stack *s, Uint8 a) { return peek8(s, a * 2) + (peek8(s, a * 2 + 1) << 8); }
void   mempoke16(Uxn *u, Uint16 a, Uint16 b) { mempoke8(u, a, b >> 8); mempoke8(u, a + 1, b); }
Uint16 mempeek16(Uxn *u, Uint16 a) { return (mempeek8(u, a) << 8) + mempeek8(u, a + 1); }
/* Stack */
void op_brk(Uxn *u) { setflag(&u->status, FLAG_HALT, 1); }
void op_nop(Uxn *u) { (void)u; }
void op_lit(Uxn *u) { setflag(&u->status, FLAG_LIT1, 1); }
void op_pop(Uxn *u) { pop8(u->src); }
void op_dup(Uxn *u) { push8(u->src, peek8(u->src, 0)); }
void op_swp(Uxn *u) { Uint8 b = pop8(u->src), a = pop8(u->src); push8(u->src, b); push8(u->src, a); }
void op_ovr(Uxn *u) { push8(u->src, peek8(u->src, 1)); }
void op_rot(Uxn *u) { Uint8 c = pop8(u->src), b = pop8(u->src), a = pop8(u->src); push8(u->src, b); push8(u->src, c); push8(u->src, a); }
/* Logic */
void op_equ(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b == a); }
void op_neq(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b != a); }
void op_gth(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b > a); }
void op_lth(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b < a); }
void op_gts(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, (Sint8)b > (Sint8)a); }
void op_lts(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, (Sint8)b < (Sint8)a); }
/* Memory */
void op_pek(Uxn *u) { Uint8 a = pop8(u->src); push8(u->src, mempeek8(u, a)); }
void op_pok(Uxn *u) { Uint8 a = pop8(u->src); Uint8 b = pop8(u->src); mempoke8(u, a, b); }
void op_ldr(Uxn *u) { Uint8 a = pop8(u->src); push16(u->src, mempeek16(u, a)); }
void op_str(Uxn *u) { Uint8 a = pop8(u->src); Uint16 b = pop16(u->src); mempoke16(u, a, b); }
void op_jmp(Uxn *u) { Uint8 a = pop8(u->src); u->ram.ptr += (Sint8)a; }
void op_jnz(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); if (b) u->ram.ptr += (Sint8)a; }
void op_jsr(Uxn *u) { Uint8 a = pop8(u->src); push16(u->dst, u->ram.ptr); u->ram.ptr += (Sint8)a; }
void op_sth(Uxn *u) { Uint8 a = pop8(u->src); push8(u->dst, a); }
/* Arithmetic */
void op_add(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b + a); }
void op_sub(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b - a); }
void op_mul(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b * a); }
void op_div(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b / a); }
void op_and(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b & a); }
void op_ora(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b | a); }
void op_eor(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b ^ a); }
void op_sft(Uxn *u) { Uint8 a = pop8(u->src), b = pop8(u->src); push8(u->src, b >> (a & 0x07) << ((a & 0x70) >> 4)); }
/* Stack */
void op_lit16(Uxn *u) { setflag(&u->status, FLAG_LIT2, 1); }
void op_nop16(Uxn *u) { printf("%04x\n", pop16(u->src)); }
void op_pop16(Uxn *u) { pop16(u->src); }
void op_dup16(Uxn *u) { push16(u->src, peek16(u->src, 0)); }
void op_swp16(Uxn *u) { Uint16 b = pop16(u->src), a = pop16(u->src); push16(u->src, b); push16(u->src, a); }
void op_ovr16(Uxn *u) { push16(u->src, peek16(u->src, 1)); }
void op_rot16(Uxn *u) { Uint16 c = pop16(u->src), b = pop16(u->src), a = pop16(u->src); push16(u->src, b); push16(u->src, c); push16(u->src, a); }
/* Logic(16-bits) */
void op_equ16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, b == a); }
void op_neq16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, b != a); }
void op_gth16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, b > a); }
void op_lth16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, b < a); }
void op_gts16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, (Sint16)b > (Sint16)a); }
void op_lts16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push8(u->src, (Sint16)b < (Sint16)a); }
/* Memory(16-bits) */
void op_pek16(Uxn *u) { Uint16 a = pop16(u->src); push8(u->src, mempeek8(u, a)); }
void op_pok16(Uxn *u) { Uint16 a = pop16(u->src); Uint8 b = pop8(u->src); mempoke8(u, a, b); }
void op_ldr16(Uxn *u) { Uint16 a = pop16(u->src); push16(u->src, mempeek16(u, a)); }
void op_str16(Uxn *u) { Uint16 a = pop16(u->src); Uint16 b = pop16(u->src); mempoke16(u, a, b); }
void op_jmp16(Uxn *u) { u->ram.ptr = pop16(u->src); }
void op_jnz16(Uxn *u) { Uint16 a = pop16(u->src); Uint8 b = pop8(u->src); if (b) u->ram.ptr = a; }
void op_jsr16(Uxn *u) { push16(u->dst, u->ram.ptr); u->ram.ptr = pop16(u->src); }
void op_sth16(Uxn *u) { Uint16 a = pop16(u->src); push16(u->dst, a); }
/* Arithmetic(16-bits) */
void op_add16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b + a); }
void op_sub16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b - a); }
void op_mul16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b * a); }
void op_div16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b / a); }
void op_and16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b & a); }
void op_ora16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b | a); }
void op_eor16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b ^ a); }
void op_sft16(Uxn *u) { Uint16 a = pop16(u->src), b = pop16(u->src); push16(u->src, b >> (a & 0x000f) << ((a & 0x00f0) >> 4)); }

void (*ops[])(Uxn *u) = {
	op_brk, op_nop, op_lit, op_pop, op_dup, op_swp, op_ovr, op_rot,
	op_equ, op_neq, op_gth, op_lth, op_gts, op_lts, op_nop, op_nop,
	op_pek, op_pok, op_ldr, op_str, op_jmp, op_jnz, op_jsr, op_sth, 
	op_add, op_sub, op_mul, op_div, op_and, op_ora, op_eor, op_sft,
	/* 16-bit */
	op_brk,   op_nop16, op_lit16, op_pop16, op_dup16, op_swp16, op_ovr16, op_rot16,
	op_equ16, op_neq16, op_gth16, op_lth16, op_gts16, op_lts16, op_nop,   op_nop, 
	op_pek16, op_pok16, op_ldr16, op_str16, op_jmp16, op_jnz16, op_jsr16, op_sth16, 
	op_add16, op_sub16, op_mul16, op_div16, op_and16, op_ora16, op_eor16, op_sft16
};

/* clang-format on */

#pragma mark - Core

int
haltuxn(Uxn *u, char *name, int id)
{
	printf("Halted: %s#%04x, at 0x%04x\n", name, id, u->ram.ptr);
	return 0;
}

void
lituxn(Uxn *u, Uint8 instr)
{
	push8(u->src, instr);
	if(getflag(&u->status, FLAG_LIT2)) {
		setflag(&u->status, FLAG_LIT2, 0);
		setflag(&u->status, FLAG_LIT1, 1);
	} else if(getflag(&u->status, FLAG_LIT1))
		setflag(&u->status, FLAG_LIT1, 0);
}

void
opcuxn(Uxn *u, Uint8 instr)
{
	Uint8 op = instr & 0x3f, freturn = instr & 0x40;
	u->src = freturn ? &u->rst : &u->wst;
	u->dst = freturn ? &u->wst : &u->rst;
	(*ops[op])(u);
}

int
stepuxn(Uxn *u, Uint8 instr)
{
	if(getflag(&u->status, FLAG_LIT2) || getflag(&u->status, FLAG_LIT1))
		lituxn(u, instr);
	else
		opcuxn(u, instr);
	if(u->wst.error)
		return haltuxn(u, u->wst.error == 1 ? "Working-stack underflow" : "Working-stack overflow", instr);
	if(u->rst.error)
		return haltuxn(u, u->rst.error == 1 ? "Return-stack underflow" : "Return-stack overflow", instr);
	return 1;
}

int
evaluxn(Uxn *u, Uint16 vec)
{
	u->status = 0;
	u->ram.ptr = vec;
	u->wst.error = 0;
	u->rst.error = 0;
	setflag(&u->status, FLAG_HALT, 0);
	while(!(u->status & FLAG_HALT)) {
		Uint8 instr = u->ram.dat[u->ram.ptr++];
		if(!stepuxn(u, instr))
			return 0;
	}
	return 1;
}

int
bootuxn(Uxn *u)
{
	size_t i;
	char *cptr = (char *)u;
	for(i = 0; i < sizeof(*u); i++)
		cptr[i] = 0;
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb")))
		return haltuxn(u, "Missing input rom.", 0);
	fread(u->ram.dat + PAGE_VECTORS, sizeof(u->ram.dat) - PAGE_VECTORS, 1, f);
	printf("Uxn loaded[%s].\n", filepath);
	return 1;
}

Device *
portuxn(Uxn *u, Uint8 id, char *name, Uint8 (*pofn)(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1))
{
	Device *d = &u->dev[id];
	d->addr = PAGE_DEVICE + id * 0x10;
	d->poke = pofn;
	printf("Device #%d: %s, at 0x%04x \n", id, name, d->addr);
	return d;
}
