#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include "cpu.h"

#define FLAG_HALT 0x01
#define FLAG_SHORT 0x02
#define FLAG_SIGN 0x04
#define FLAG_COND 0x08

Computer cpu;

#pragma mark - Helpers

void
setflag(char flag, int b)
{
	if(b)
		cpu.status |= flag;
	else
		cpu.status &= (~flag);
}

int
getflag(char flag)
{
	return cpu.status & flag;
}

void
echos(Stack8 *s, Uint8 len, char *name)
{
	int i;
	printf("\n%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x%c", s->dat[i], s->ptr == i ? '<' : ' ');
	}
	printf("\n\n");
}

void
echom(Memory *m, Uint8 len, char *name)
{
	int i;
	printf("\n%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", m->dat[i]);
	}
	printf("\n\n");
}

#pragma mark - Operations

/* clang-format off */

Uint16 bytes2short(Uint8 a, Uint8 b) { return (a << 8) + b; }
Uint16 mempeek16(Uint16 s) { return (cpu.ram.dat[s] << 8) + (cpu.ram.dat[s+1] & 0xff); }
void wspush8(Uint8 b) { cpu.wst.dat[cpu.wst.ptr++] = b; }
void wspush16(Uint16 s) { wspush8(s >> 8); wspush8(s & 0xff); }
Uint8 wspop8(void) { return cpu.wst.dat[--cpu.wst.ptr]; }
Uint16 wspop16(void) { return wspop8() + (wspop8() << 8); }
Uint8 wspeek8(Uint8 o) { return cpu.wst.dat[cpu.wst.ptr - o]; }
Uint16 wspeek16(Uint8 o) { return bytes2short(cpu.wst.dat[cpu.wst.ptr - o], cpu.wst.dat[cpu.wst.ptr - o + 1]); }
Uint16 rspop16(void) { return cpu.rst.dat[--cpu.rst.ptr]; }
void rspush16(Uint16 a) { cpu.rst.dat[cpu.rst.ptr++] = a; }

/* I/O */
void op_brk() { setflag(FLAG_HALT, 1); }
void op_lit() { cpu.literal += cpu.ram.dat[cpu.ram.ptr++]; }
void op_nop() { printf("NOP");}
void op_ldr() { wspush8(cpu.ram.dat[wspop16()]); }
void op_str() { cpu.ram.dat[wspop16()] = wspop8(); }
/* Logic */
void op_jmp() { cpu.ram.ptr = wspop16(); }
void op_jsr() { rspush16(cpu.ram.ptr); cpu.ram.ptr = wspop16(); }
void op_rts() {	cpu.ram.ptr = rspop16(); }
/* Stack */
void op_pop() { wspop8(); }
void op_dup() { wspush8(wspeek8(1)); }
void op_swp() { Uint8 b = wspop8(), a = wspop8(); wspush8(b); wspush8(a); }
void op_ovr() { Uint8 a = wspeek8(2); wspush8(a); }
void op_rot() { Uint8 c = wspop8(),b = wspop8(),a = wspop8(); wspush8(b); wspush8(c); wspush8(a); }
void op_and() { Uint8 a = wspop8(), b = wspop8(); wspush8(a & b); }
void op_ora() { Uint8 a = wspop8(), b = wspop8(); wspush8(a | b); }
void op_rol() { Uint8 a = wspop8(), b = wspop8(); wspush8(a << b); }
/* Arithmetic */
void op_add() { Uint8 a = wspop8(), b = wspop8(); wspush8(b + a); }
void op_sub() { Uint8 a = wspop8(), b = wspop8(); wspush8(b - a); }
void op_mul() { Uint8 a = wspop8(), b = wspop8(); wspush8(b * a); }
void op_div() { Uint8 a = wspop8(), b = wspop8(); wspush8(b / a); }
void op_equ() { Uint8 a = wspop8(), b = wspop8(); wspush8(b == a); }
void op_neq() { Uint8 a = wspop8(), b = wspop8(); wspush8(b != a); }
void op_gth() { Uint8 a = wspop8(), b = wspop8(); wspush8(b > a); }
void op_lth() { Uint8 a = wspop8(), b = wspop8(); wspush8(b < a); }
/* Stack(16-bits) */
void op_pop16() { wspop16(); }
void op_dup16() { wspush16(wspeek16(2)); }
void op_swp16() { Uint16 b = wspop16(), a = wspop16(); wspush16(b); wspush16(a); }
void op_ovr16() { Uint16 a = wspeek16(4); wspush16(a); }
void op_rot16() { Uint16 c = wspop16(), b = wspop16(), a = wspop16(); wspush16(b); wspush16(c); wspush16(a); }
void op_and16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a & b); }
void op_ora16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a | b); }
void op_rol16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a << b); }
/* Arithmetic(16-bits) */
void op_add16() { Uint16 a = wspop16(), b = wspop16(); wspush16(b + a); }
void op_sub16() { Uint16 a = wspop16(), b = wspop16(); wspush16(b - a); }
void op_mul16() { Uint16 a = wspop16(), b = wspop16(); wspush16(b * a); }
void op_div16() { Uint16 a = wspop16(), b = wspop16(); wspush16(b / a); }
void op_equ16() { Uint16 a = wspop16(), b = wspop16(); wspush8(b == a); }
void op_neq16() { Uint16 a = wspop16(), b = wspop16(); wspush8(b != a); }
void op_gth16() { Uint16 a = wspop16(), b = wspop16(); wspush8(b > a); }
void op_lth16() { Uint16 a = wspop16(), b = wspop16(); wspush8(b < a); }

void (*ops[])() = {
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
error(char *name, int id)
{
	printf("Error: %s[%04x], at 0x%04x\n", name, id, cpu.counter);
	return 0;
}

int
doliteral(Uint8 instr)
{
	if(cpu.wst.ptr >= 255)
		return error("Stack overflow", instr);
	wspush8(instr);
	cpu.literal--;
	return 1;
}

int
dodevices(void) /* experimental */
{
	if(cpu.ram.dat[0xfff1]) {
		printf("%c", cpu.ram.dat[0xfff1]);
		cpu.ram.dat[0xfff1] = 0x00;
	}
	return 1;
}

int
doopcode(Uint8 instr)
{
	Uint8 op = instr & 0x1f;
	setflag(FLAG_SHORT, (instr >> 5) & 1);
	setflag(FLAG_SIGN, (instr >> 6) & 1); /* usused */
	setflag(FLAG_COND, (instr >> 7) & 1);
	if(getflag(FLAG_SHORT))
		op += 16;
	if(cpu.wst.ptr < opr[op][0])
		return error("Stack underflow", op);
	if(cpu.wst.ptr + opr[op][1] - opr[op][0] >= 255)
		return error("Stack overflow", instr);
	if(!getflag(FLAG_COND) || (getflag(FLAG_COND) && wspop8()))
		(*ops[op])();
	dodevices();
	return 1;
}

int
eval(void)
{
	Uint8 instr = cpu.ram.dat[cpu.ram.ptr++];
	if(cpu.literal > 0)
		return doliteral(instr);
	else
		return doopcode(instr);
	return 1;
}

int
load(FILE *f)
{
	fread(cpu.ram.dat, sizeof(cpu.ram.dat), 1, f);
	return 1;
}

void
echof(void)
{
	printf("ended @ %d steps | hf: %x sf: %x sf: %x cf: %x\n",
		cpu.counter,
		getflag(FLAG_HALT) != 0,
		getflag(FLAG_SHORT) != 0,
		getflag(FLAG_SIGN) != 0,
		getflag(FLAG_COND) != 0);
}

int
boot(void)
{
	cpu.vreset = mempeek16(0xfffa);
	cpu.vframe = mempeek16(0xfffc);
	cpu.verror = mempeek16(0xfffe);
	/* eval reset */
	cpu.ram.ptr = cpu.vreset;
	setflag(FLAG_HALT, 0);
	while(!(cpu.status & FLAG_HALT) && eval())
		cpu.counter++;
	/*eval frame */
	cpu.ram.ptr = cpu.vframe;
	setflag(FLAG_HALT, 0);
	while(!(cpu.status & FLAG_HALT) && eval())
		cpu.counter++;
	return 1;
}
