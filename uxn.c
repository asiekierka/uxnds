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
void   mempoke8(Memory *m, Uint16 a, Uint8 b) { m->dat[a] = b; }
Uint8  mempeek8(Memory *m, Uint16 a) { return m->dat[a]; }
void   mempoke16(Memory *m, Uint16 a, Uint16 b) { mempoke8(m, a, b >> 8); mempoke8(m, a + 1, b); }
Uint16 mempeek16(Memory *m, Uint16 a) { return (mempeek8(m, a) << 8) + mempeek8(m, a + 1); }
void   push8(Stack *s, Uint8 a) { s->dat[s->ptr++] = a; }
Uint8  pop8(Stack *s) { return s->dat[--s->ptr]; }
Uint8  peek8(Stack *s, Uint8 a) { return s->dat[s->ptr - a - 1]; }
void   push16(Stack *s, Uint16 a) { push8(s, a >> 8); push8(s, a); }
Uint16 pop16(Stack *s) { return pop8(s) + (pop8(s) << 8); }
Uint16 peek16(Stack *s, Uint8 a) { return peek8(s, a * 2) + (peek8(s, a * 2 + 1) << 8); }
/* I/O */
void op_brk(Uxn *u) { setflag(&u->status, FLAG_HALT, 1); }
void op_lit(Uxn *u) { u->literal += 1; }
void op_nop(Uxn *u) { printf("0x%02x \n", pop8(&u->wst)); fflush(stdout); }
void op_ior(Uxn *u) { Device *dev = &u->dev[mempeek8(&u->ram, u->devr)]; if(dev) push8(&u->wst, dev->read(dev, &u->ram, pop8(&u->wst))); }
void op_iow(Uxn *u) { Uint8 a = pop8(&u->wst); Device *dev = &u->dev[mempeek8(&u->ram, u->devw)]; if(dev) dev->write(dev, &u->ram, a); }
void op_ldr(Uxn *u) { Uint16 a = pop16(&u->wst); push8(&u->wst, mempeek8(&u->ram, a)); }
void op_str(Uxn *u) { Uint16 a = pop16(&u->wst); Uint8 b = pop8(&u->wst); mempoke8(&u->ram, a, b); }
/* Logic */
void op_jmp(Uxn *u) { u->ram.ptr = pop16(&u->wst); }
void op_jsr(Uxn *u) { push16(&u->rst, u->ram.ptr); u->ram.ptr = pop16(&u->wst); }
void op_rts(Uxn *u) { u->ram.ptr = pop16(&u->rst); }
void op_and(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, b & a); }
void op_ora(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, b | a); }
void op_rol(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, b << a); }
void op_ror(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, b >> a); }
/* Stack */
void op_pop(Uxn *u) { pop8(&u->wst); }
void op_dup(Uxn *u) { push8(&u->wst, peek8(&u->wst, 0)); }
void op_swp(Uxn *u) { Uint8 b = pop8(&u->wst), a = pop8(&u->wst); push8(&u->wst, b); push8(&u->wst, a); }
void op_ovr(Uxn *u) { push8(&u->wst, peek8(&u->wst, 1)); }
void op_rot(Uxn *u) { Uint8 c = pop8(&u->wst), b = pop8(&u->wst), a = pop8(&u->wst); push8(&u->wst, b); push8(&u->wst, c); push8(&u->wst, a); }
void op_wsr(Uxn *u) { Uint8 a = pop8(&u->wst); push8(&u->rst, a); u->balance++; }
void op_rsw(Uxn *u) { Uint8 a = pop8(&u->rst); push8(&u->wst, a); u->balance--; }
/* Arithmetic */
void op_add(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b + (Sint8)a : b + a); }
void op_sub(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b - (Sint8)a : b - a); }
void op_mul(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b * (Sint8)a : b * a); }
void op_div(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b / (Sint8)a : b / a); }
void op_equ(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b == (Sint8)a : b == a); }
void op_neq(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b != (Sint8)a : b != a); }
void op_gth(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b > (Sint8)a : b > a); }
void op_lth(Uxn *u) { Uint8 a = pop8(&u->wst), b = pop8(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint8)b < (Sint8)a : b < a); }
/* --- */
void op_lit16(Uxn *u) { u->literal += 2; }
void op_nop16(Uxn *u) { printf("%04x\n", pop16(&u->wst)); }
void op_ior16(Uxn *u) { Uint8 a = pop8(&u->wst); Device *dev = &u->dev[mempeek8(&u->ram, u->devr)]; if(dev) push16(&u->wst, (dev->read(dev, &u->ram, a) << 8) + dev->read(dev, &u->ram, a + 1)); }
void op_iow16(Uxn *u) { Uint8 a = pop8(&u->wst); Uint8 b = pop8(&u->wst); Device *dev = &u->dev[mempeek8(&u->ram, u->devw)]; if(dev) { dev->write(dev, &u->ram, b); dev->write(dev, &u->ram, a); } }
void op_ldr16(Uxn *u) { Uint16 a = pop16(&u->wst); push16(&u->wst, mempeek16(&u->ram, a)); }
void op_str16(Uxn *u) { Uint16 a = pop16(&u->wst); Uint16 b = pop16(&u->wst); mempoke16(&u->ram, a, b); }
void op_and16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, b & a); }
void op_ora16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, b | a); }
void op_rol16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, b << a); }
void op_ror16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, b >> a); }
/* Stack(16-bits) */
void op_pop16(Uxn *u) { pop16(&u->wst); }
void op_dup16(Uxn *u) { push16(&u->wst, peek16(&u->wst, 0)); }
void op_swp16(Uxn *u) { Uint16 b = pop16(&u->wst), a = pop16(&u->wst); push16(&u->wst, b); push16(&u->wst, a); }
void op_ovr16(Uxn *u) { push16(&u->wst, peek16(&u->wst, 1)); }
void op_rot16(Uxn *u) { Uint16 c = pop16(&u->wst), b = pop16(&u->wst), a = pop16(&u->wst); push16(&u->wst, b); push16(&u->wst, c); push16(&u->wst, a); }
void op_wsr16(Uxn *u) { Uint16 a = pop16(&u->wst); push16(&u->rst, a); u->balance += 2; }
void op_rsw16(Uxn *u) { Uint16 a = pop16(&u->rst); push16(&u->wst, a); u->balance -= 2; }
/* Arithmetic(16-bits) */
void op_add16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b + (Sint16)a : b + a); }
void op_sub16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b - (Sint16)a : b - a); }
void op_mul16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b * (Sint16)a : b * a); }
void op_div16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push16(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b / (Sint16)a : b / a); }
void op_equ16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b == (Sint16)a : b == a); }
void op_neq16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b != (Sint16)a : b != a); }
void op_gth16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b > (Sint16)a : b > a); }
void op_lth16(Uxn *u) { Uint16 a = pop16(&u->wst), b = pop16(&u->wst); push8(&u->wst, getflag(&u->status, FLAG_SIGN) ? (Sint16)b < (Sint16)a : b < a); }

void (*ops[])(Uxn *u) = {
	op_brk, op_nop, op_lit, op_nop, op_ior, op_iow, op_ldr, op_str, 
	op_jmp, op_jsr, op_nop, op_rts, op_and, op_ora, op_rol, op_ror, 
	op_pop, op_dup, op_swp, op_ovr, op_rot, op_wsr, op_rsw, op_nop,
	op_add, op_sub, op_mul, op_div, op_equ, op_neq, op_gth, op_lth,
	/* 16-bit */
	op_brk,   op_nop16, op_lit16, op_nop,   op_ior16, op_iow16, op_ldr16, op_str16, 
	op_jmp,   op_jsr,   op_nop,   op_rts,   op_and16, op_ora16, op_rol16, op_ror16, 
	op_pop16, op_dup16, op_swp16, op_ovr16, op_rot16, op_wsr16, op_rsw16, op_nop,
	op_add16, op_sub16, op_mul16, op_div16, op_equ16, op_neq16, op_gth16, op_lth16
};

Uint8 opr[][2] = { 
	{0,0}, {0,0}, {0,0}, {0,0}, {1,1}, {1,0}, {2,1}, {3,0},
	{2,0}, {2,0}, {0,0}, {0,0}, {2,1}, {2,1}, {2,1}, {2,1},
	{1,0}, {1,2}, {2,2}, {2,3}, {3,3}, {1,0}, {0,1}, {2,1},
	{2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1},
	/* 16-bit */
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, /* TODO */
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, /* TODO */
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,0}, {0,1}, {0,0}, /* TODO */
	{4,2}, {4,2}, {4,2}, {4,2}, {4,2}, {4,2}, {4,2}, {4,2}
};

/* clang-format on */

int
haltuxn(Uxn *u, char *name, int id)
{
	printf("Halted: %s#%04x, at 0x%04x\n", name, id, u->counter);
	return 0;
}

int
lituxn(Uxn *u, Uint8 instr)
{
	if(u->wst.ptr >= 255)
		return haltuxn(u, "Stack overflow", instr);
	push8(&u->wst, instr);
	u->literal--;
	return 1;
}

int
opcuxn(Uxn *u, Uint8 instr)
{
	Uint8 op = instr & 0x1f;
	setflag(&u->status, FLAG_SHORT, (instr >> 5) & 1);
	setflag(&u->status, FLAG_SIGN, (instr >> 6) & 1);
	setflag(&u->status, FLAG_COND, (instr >> 7) & 1);
	if((op == 0x09 || op == 0x0b) && u->balance)
		return haltuxn(u, "Stack unbalance", op);
	if(getflag(&u->status, FLAG_SHORT))
		op += 32;
	if(u->wst.ptr < opr[op][0])
		return haltuxn(u, "Stack underflow", op);
	if(u->wst.ptr + opr[op][1] - opr[op][0] >= 255)
		return haltuxn(u, "Stack overflow", instr);
	if(!getflag(&u->status, FLAG_COND) || (getflag(&u->status, FLAG_COND) && pop8(&u->wst)))
		(*ops[op])(u);
	return 1;
}

int
stepuxn(Uxn *u, Uint8 instr)
{
	if(u->literal > 0)
		return lituxn(u, instr);
	else
		return opcuxn(u, instr);
}

int
evaluxn(Uxn *u, Uint16 vec)
{
	u->literal = 0;
	u->ram.ptr = vec;
	setflag(&u->status, FLAG_HALT, 0);
	while(!(u->status & FLAG_HALT)) {
		Uint8 instr = u->ram.dat[u->ram.ptr++];
		if(!stepuxn(u, instr))
			return 0;
		u->counter++;
	}
	return 1;
}

int
bootuxn(Uxn *u)
{
	size_t i;
	char *cptr = (char *)&u;
	for(i = 0; i < sizeof u; i++)
		cptr[i] = 0;
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb")))
		return haltuxn(u, "Missing input rom.", 0);
	fread(u->ram.dat, sizeof(u->ram.dat), 1, f);
	u->devr = 0xfff8;
	u->devw = 0xfff9;
	u->vreset = mempeek16(&u->ram, 0xfffa);
	u->vframe = mempeek16(&u->ram, 0xfffc);
	u->verror = mempeek16(&u->ram, 0xfffe);
	printf("Uxn loaded[%s] vrst:%04x vfrm:%04x verr:%04x.\n",
		filepath,
		u->vreset,
		u->vframe,
		u->verror);
	return 1;
}

Device *
portuxn(Uxn *u, char *name, Uint8 (*rfn)(Device *, Memory *, Uint8), Uint8 (*wfn)(Device *, Memory *, Uint8))
{
	Device *d = &u->dev[u->devices++];
	d->read = rfn;
	d->write = wfn;
	d->ptr = 0;
	printf("Device #%d: %s \n", u->devices - 1, name);
	return d;
}
