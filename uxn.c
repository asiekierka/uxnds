#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define FLAG_HALT 0x01
#define FLAG_SHORT 0x02
#define FLAG_SIGN 0x04
#define FLAG_TRAPS 0x08

typedef unsigned char Uint8;
typedef unsigned short Uint16;

typedef struct {
	Uint8 ptr;
	Uint8 dat[256];
} Stack;

typedef struct {
	Uint8 ptr;
	Uint16 dat[256];
} Stack16;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct {
	Uint8 literal, status;
	Uint16 counter, vreset, vframe, verror;
	Stack wst;
	Stack16 rst;
	Memory rom, ram;
} Computer;

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
echos(Stack *s, Uint8 len, char *name)
{
	int i;
	printf("%s\n", name);
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
	printf("%s\n", name);
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
Uint8 rampeek8(Uint16 s) { return cpu.ram.dat[s] & 0xff; }
Uint8 mempeek8(Uint16 s) { return cpu.rom.dat[s]; }
Uint16 mempeek16(Uint16 s) { return (cpu.rom.dat[s] << 8) + (cpu.rom.dat[s+1] & 0xff); }
void wspush8(Uint8 b) { cpu.wst.dat[cpu.wst.ptr++] = b; }
void wspush16(Uint16 s) { wspush8(s >> 8); wspush8(s & 0xff); }
Uint8 wspop8(void) { return cpu.wst.dat[--cpu.wst.ptr]; }
Uint16 wspop16(void) { return wspop8() + (wspop8() << 8); }
Uint8 wspeek8(Uint8 o) { return cpu.wst.dat[cpu.wst.ptr - o]; }
Uint16 wspeek16(Uint8 o) { return bytes2short(cpu.wst.dat[cpu.wst.ptr - o], cpu.wst.dat[cpu.wst.ptr - o + 1]); }
Uint16 rspop16(void) { return cpu.rst.dat[--cpu.rst.ptr]; }
void rspush16(Uint16 a) { cpu.rst.dat[cpu.rst.ptr++] = a; }

/* new flexy pop/push */

void op_brk() { setflag(FLAG_HALT, 1); }
void op_rts() {	cpu.rom.ptr = rspop16(); }
void op_lit() { cpu.literal += cpu.rom.dat[cpu.rom.ptr++]; }

void op_drp() { wspop8(); }
void op_dup() { wspush8(wspeek8(1)); }
void op_swp() { Uint8 b = wspop8(), a = wspop8(); wspush8(b); wspush8(a); }
void op_ovr() { Uint8 a = wspeek8(2); wspush8(a); }
void op_rot() { Uint8 c = wspop8(),b = wspop8(),a = wspop8(); wspush8(b); wspush8(c); wspush8(a); }

void op_jmu() { cpu.rom.ptr = wspop16(); }
void op_jsu() { rspush16(cpu.rom.ptr); cpu.rom.ptr = wspop16(); }
void op_jmc() { Uint8 a = wspop8(); if(a) op_jmu(); }
void op_jsc() { Uint8 a = wspop8(); if(a) op_jsu(); }

void op_equ() { Uint8 a = wspop8(), b = wspop8(); wspush8(a == b); }
void op_neq() { Uint8 a = wspop8(), b = wspop8(); wspush8(a != b); }
void op_gth() { Uint8 a = wspop8(), b = wspop8(); wspush8(a < b); }
void op_lth() { Uint8 a = wspop8(), b = wspop8(); wspush8(a > b); }
void op_and() { Uint8 a = wspop8(), b = wspop8(); wspush8(a & b); }
void op_ora() { Uint8 a = wspop8(), b = wspop8(); wspush8(a | b); }
void op_rol() { Uint8 a = wspop8(), b = wspop8(); wspush8(a << b); }
void op_ror() { Uint8 a = wspop8(), b = wspop8(); wspush8(a >> b); }
void op_add() { Uint8 a = wspop8(), b = wspop8(); wspush8(a + b); }
void op_sub() { Uint8 a = wspop8(), b = wspop8(); wspush8(a - b); }
void op_mul() { Uint8 a = wspop8(), b = wspop8(); wspush8(a * b); }
void op_div() { Uint8 a = wspop8(), b = wspop8(); wspush8(a / b); }
void op_ldr() { wspush8(cpu.ram.dat[wspop16()]); }
void op_str() { cpu.ram.dat[wspop16()] = wspop8(); }
void op_pek() { wspush8(cpu.rom.dat[wspop16()]); }
void op_pok() { printf("TODO:\n");}

void op_drp16() { wspop16(); }
void op_dup16() { wspush16(wspeek16(2)); }
void op_swp16() { Uint16 b = wspop16(), a = wspop16(); wspush16(b); wspush16(a); }
void op_ovr16() { Uint16 a = wspeek16(4); wspush16(a); }
void op_rot16() { Uint16 c = wspop16(), b = wspop16(), a = wspop16(); wspush16(b); wspush16(c); wspush16(a); }

void op_equ16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a == b); }
void op_neq16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a != b); }
void op_gth16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a < b); }
void op_lth16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a > b); }
void op_and16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a & b); }
void op_ora16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a | b); }
void op_rol16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a << b); }
void op_ror16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a >> b); }
void op_add16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a + b); }
void op_sub16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a - b); }
void op_mul16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a * b); }
void op_div16() { Uint16 a = wspop16(), b = wspop16(); wspush16(a / b); }

void (*ops8[])() = {
	op_brk, op_rts, op_lit, op_drp, op_dup, op_swp, op_ovr, op_rot, 
	op_jmu, op_jsu, op_jmc, op_jsc, op_equ, op_neq, op_gth, op_lth, 
	op_and, op_ora, op_rol, op_ror, op_add, op_sub, op_mul, op_div,
	op_ldr, op_str, op_pek, op_pok, op_brk, op_brk, op_brk, op_brk
};

void (*ops16[])() = {
	op_brk, op_rts, op_lit, op_drp16, op_dup16, op_swp16, op_ovr16, op_rot16, 
	op_jmu, op_jsu, op_jmc, op_jsc, op_equ, op_neq, op_gth, op_lth, 
	op_and16, op_ora16, op_rol16, op_ror16, op_add16, op_sub16, op_mul16, op_div16,
	op_ldr, op_str, op_pek, op_pok, op_brk, op_brk, op_brk, op_brk
};

Uint8 opr[][2] = { /* todo: 16 bits mode */
	{0,0}, {0,0}, {0,0}, {1,0}, {0,1}, {1,1}, {0,1}, {3,3},
	{2,0}, {2,0}, {2,0}, {2,0}, {2,1}, {2,1}, {2,1}, {2,1},
	{1,0}, {1,0}, {1,0}, {1,0}, {2,1}, {0,0}, {0,0}, {0,0},
	{2,1}, {3,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
};

/* clang-format on */

void
reset(void)
{
	size_t i;
	char *cptr = (char *)&cpu;
	for(i = 0; i < sizeof cpu; i++)
		cptr[i] = 0;
}

int
error(char *name, int id)
{
	printf("Error: %s[%04x], at 0x%04x\n", name, id, cpu.counter);
	return 0;
}

int
device1(Uint8 *read, Uint8 *write)
{
	printf("%c", *write);
	*write = 0;
	(void)read;
	return 0;
}

void
opc(Uint8 src, Uint8 *op)
{
	*op = src;
	*op &= ~(1 << 5);
	*op &= ~(1 << 6);
	*op &= ~(1 << 7);
}

int
eval(void)
{
	Uint8 instr = cpu.rom.dat[cpu.rom.ptr++];
	Uint8 op;
	/* when literal */
	if(cpu.literal > 0) {
		wspush8(instr);
		cpu.literal--;
		return 1;
	}
	/* when opcode */
	opc(instr, &op);
	setflag(FLAG_SHORT, (instr >> 5) & 1);
	setflag(FLAG_SIGN, (instr >> 6) & 1);
	if((instr >> 7) & 1)
		printf("Unused flag: %02x\n", instr);
	/* TODO: setflag(FLAG_B, (instr >> 6) & 1); */
	/* TODO: setflag(FLAG_C, (instr >> 7) & 1); */
	if(cpu.wst.ptr < opr[op][0])
		return error("Stack underflow", op);
	if(getflag(FLAG_SHORT))
		(*ops16[op])();
	else
		(*ops8[op])();
	cpu.counter++;
	return 1;
}

int
start(FILE *f)
{
	reset();
	fread(cpu.rom.dat, sizeof(cpu.rom.dat), 1, f);
	cpu.vreset = mempeek16(0xfffa);
	cpu.vframe = mempeek16(0xfffc);
	cpu.verror = mempeek16(0xfffe);
	/* eval reset */
	printf("\nPhase: reset\n");
	cpu.rom.ptr = cpu.vreset;
	while(!(cpu.status & FLAG_HALT) && eval()) {
		/* experimental */
		if(cpu.ram.dat[0xfff1])
			device1(&cpu.ram.dat[0xfff0], &cpu.ram.dat[0xfff1]);
		if(cpu.counter > 256)
			return error("Reached bounds", cpu.counter);
	}
	/*eval frame */
	printf("\nPhase: frame\n");
	setflag(FLAG_HALT, 0);
	cpu.rom.ptr = cpu.vframe;
	while(!(cpu.status & FLAG_HALT) && eval())
		;
	/* debug */
	printf("ended @ %d steps | ", cpu.counter);
	printf("hf: %x zf: %x cf: %x tf: %x\n",
		getflag(FLAG_HALT) != 0,
		getflag(FLAG_SHORT) != 0,
		getflag(FLAG_SIGN) != 0,
		getflag(FLAG_TRAPS) != 0);
	printf("\n");
	return 1;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	if(argc < 2)
		return error("No input.", 0);
	if(!(f = fopen(argv[1], "rb")))
		return error("Missing input.", 0);
	if(!start(f))
		return error("Program error", 0);
	/* print result */
	echos(&cpu.wst, 0x40, "stack");
	echom(&cpu.ram, 0x40, "ram");
	return 0;
}
