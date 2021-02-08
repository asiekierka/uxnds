#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "uxn.h"

Uxn cpu;

#pragma mark - Helpers

void
setflag(Uint8 *status, char flag, int b)
{
	if(b)
		*status |= flag;
	else
		*status &= (~flag);
}

int
getflag(Uint8 *status, char flag)
{
	return *status & flag;
}

#pragma mark - Operations

/* clang-format off */

Uint16 bytes2short(Uint8 a, Uint8 b) { return (a << 8) + b; }
Uint16 mempeek16(Uxn *c, Uint16 s) { return (c->ram.dat[s] << 8) + (c->ram.dat[s+1] & 0xff); }
void wspush8(Uxn *c, Uint8 b) { c->wst.dat[c->wst.ptr++] = b; }
void wspush16(Uxn *c, Uint16 s) { wspush8(c,s >> 8); wspush8(c,s & 0xff); }
Uint8 wspop8(Uxn *c) { return c->wst.dat[--c->wst.ptr]; }
Uint16 wspop16(Uxn *c) { return wspop8(c) + (wspop8(c) << 8); }
Uint8 wspeek8(Uxn *c, Uint8 o) { return c->wst.dat[c->wst.ptr - o]; }
Uint16 wspeek16(Uxn *c, Uint8 o) { return bytes2short(c->wst.dat[c->wst.ptr - o], c->wst.dat[c->wst.ptr - o + 1]); }
Uint16 rspop16(Uxn *c) { return c->rst.dat[--c->rst.ptr]; }
void rspush16(Uxn *c, Uint16 a) { c->rst.dat[c->rst.ptr++] = a; }
/* I/O */
void op_brk(Uxn *c) { setflag(&c->status,FLAG_HALT, 1); }
void op_lit(Uxn *c) { c->literal += c->ram.dat[c->ram.ptr++]; }
void op_nop(Uxn *c) { (void)c; printf("NOP");}
void op_ldr(Uxn *c) { wspush8(c, c->ram.dat[wspop16(c)]); }
void op_str(Uxn *c) { c->ram.dat[wspop16(c)] = wspop8(c); }
/* Logic */
void op_jmp(Uxn *c) { c->ram.ptr = wspop16(c); }
void op_jsr(Uxn *c) { rspush16(c, c->ram.ptr); c->ram.ptr = wspop16(c); }
void op_rts(Uxn *c) {	c->ram.ptr = rspop16(c); }
/* Stack */
void op_pop(Uxn *c) { wspop8(c); }
void op_dup(Uxn *c) { wspush8(c,wspeek8(c,1)); }
void op_swp(Uxn *c) { Uint8 b = wspop8(c), a = wspop8(c); wspush8(c,b); wspush8(c,a); }
void op_ovr(Uxn *c) { Uint8 a = wspeek8(c,2); wspush8(c,a); }
void op_rot(Uxn *c) { Uint8 c1 = wspop8(c),b = wspop8(c),a = wspop8(c); wspush8(c,b); wspush8(c, c1); wspush8(c,a); }
void op_and(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,a & b); }
void op_ora(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,a | b); }
void op_rol(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,a << b); }
/* Arithmetic */
void op_add(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b + a); }
void op_sub(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b - a); }
void op_mul(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b * a); }
void op_div(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b / a); }
void op_equ(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b == a); }
void op_neq(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b != a); }
void op_gth(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b > a); }
void op_lth(Uxn *c) { Uint8 a = wspop8(c), b = wspop8(c); wspush8(c,b < a); }
/* Stack(16-bits) */
void op_pop16(Uxn *c) { wspop16(c); }
void op_dup16(Uxn *c) { wspush16(c,wspeek16(c,2)); }
void op_swp16(Uxn *c) { Uint16 b = wspop16(c), a = wspop16(c); wspush16(c,b); wspush16(c,a); }
void op_ovr16(Uxn *c) { Uint16 a = wspeek16(c, 4); wspush16(c,a); }
void op_rot16(Uxn *c) { Uint16 c1 = wspop16(c), b = wspop16(c), a = wspop16(c); wspush16(c,b); wspush16(c, c1); wspush16(c,a); }
void op_and16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,a & b); }
void op_ora16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,a | b); }
void op_rol16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,a << b); }
/* Arithmetic(16-bits) */
void op_add16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,b + a); }
void op_sub16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,b - a); }
void op_mul16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,b * a); }
void op_div16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush16(c,b / a); }
void op_equ16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush8(c,b == a); }
void op_neq16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush8(c,b != a); }
void op_gth16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush8(c,b > a); }
void op_lth16(Uxn *c) { Uint16 a = wspop16(c), b = wspop16(c); wspush8(c,b < a); }

void (*ops[])(Uxn *c) = {
	op_brk, op_lit, op_nop, op_nop, op_nop, op_nop, op_ldr, op_str, 
	op_jmp, op_jsr, op_nop, op_rts, op_nop, op_nop, op_nop, op_nop, 
	op_pop, op_dup, op_swp, op_ovr, op_rot, op_and, op_ora, op_rol,
	op_add, op_sub, op_mul, op_div, op_equ, op_neq, op_gth, op_lth,
	op_pop16, op_dup16, op_swp16, op_ovr16, op_rot16, op_and16, op_ora16, op_rol16,
	op_add16, op_sub16, op_mul16, op_div16, op_equ16, op_neq16, op_gth16, op_lth16
};

Uint8 opr[][2] = { 
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {2,1}, {3,0},
	{2,0}, {2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{1,0}, {1,2}, {2,2}, {3,3}, {3,3}, {2,1}, {2,1}, {2,1},
	{2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, /* TODO */
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}  /* TODO */
};

/* clang-format on */

int
error(Uxn *c, char *name, int id)
{
	printf("Error: %s#%04x, at 0x%04x\n", name, id, c->counter);
	return 0;
}

int
doliteral(Uxn *c, Uint8 instr)
{
	if(c->wst.ptr >= 255)
		return error(c, "Stack overflow", instr);
	wspush8(c, instr);
	c->literal--;
	return 1;
}

int
dodevices(Uxn *c) /* experimental */
{
	if(c->ram.dat[0xfff1]) {
		printf("%c", c->ram.dat[0xfff1]);
		c->ram.dat[0xfff1] = 0x00;
	}
	return 1;
}

int
doopcode(Uxn *c, Uint8 instr)
{
	Uint8 op = instr & 0x1f;
	setflag(&c->status, FLAG_SHORT, (instr >> 5) & 1);
	setflag(&c->status, FLAG_SIGN, (instr >> 6) & 1); /* usused */
	setflag(&c->status, FLAG_COND, (instr >> 7) & 1);
	if(getflag(&c->status, FLAG_SHORT))
		op += 16;
	if(c->wst.ptr < opr[op][0])
		return error(c, "Stack underflow", op);
	if(c->wst.ptr + opr[op][1] - opr[op][0] >= 255)
		return error(c, "Stack overflow", instr);
	if(!getflag(&c->status, FLAG_COND) || (getflag(&c->status, FLAG_COND) && wspop8(c)))
		(*ops[op])(c);
	dodevices(c);
	return 1;
}

int
eval(Uxn *c)
{
	Uint8 instr = c->ram.dat[c->ram.ptr++];
	if(c->literal > 0)
		return doliteral(c, instr);
	else
		return doopcode(c, instr);
	return 1;
}

int
load(Uxn *c, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb")))
		return error(c, "Missing input.", 0);
	fread(c->ram.dat, sizeof(c->ram.dat), 1, f);
	return 1;
}

void
reset(Uxn *c)
{
	size_t i;
	char *cptr = (char *)c;
	for(i = 0; i < sizeof c; i++)
		cptr[i] = 0;
}

int
boot(Uxn *c)
{
	reset(c);
	c->vreset = mempeek16(c, 0xfffa);
	c->vframe = mempeek16(c, 0xfffc);
	c->verror = mempeek16(c, 0xfffe);
	/* eval reset */
	c->ram.ptr = c->vreset;
	setflag(&c->status, FLAG_HALT, 0);
	while(!(c->status & FLAG_HALT) && eval(c))
		c->counter++;
	/* eval frame */
	c->ram.ptr = c->vframe;
	setflag(&c->status, FLAG_HALT, 0);
	while(!(c->status & FLAG_HALT) && eval(c))
		c->counter++;
	return 1;
}
