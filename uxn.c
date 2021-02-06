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
#define FLAG_ZERO 0x02
#define FLAG_CARRY 0x04
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
Uint8 wspop8(void) { return cpu.wst.dat[--cpu.wst.ptr]; }
Uint16 wspop16(void) { return wspop8() + (wspop8() << 8); }
Uint8 wspeek8(void) { return cpu.wst.dat[cpu.wst.ptr - 1]; }
Uint16 rspop16(void) { return cpu.rst.dat[--cpu.rst.ptr]; }
void rspush16(Uint16 a) { cpu.rst.dat[cpu.rst.ptr++] = a; }

void op_brk() { setflag(FLAG_HALT, 1); }
void op_rts() {	cpu.rom.ptr = rspop16(); }
void op_lit() { cpu.literal += cpu.rom.dat[cpu.rom.ptr++]; }
void op_drp() { wspop8(); }
void op_dup() { wspush8(wspeek8()); }
void op_swp() { Uint8 b = wspop8(), a = wspop8(); wspush8(b); wspush8(a); }
void op_ovr() { wspush8(cpu.wst.dat[cpu.wst.ptr - 2]); }
void op_rot() { Uint8 c = wspop8(),b = wspop8(),a = wspop8(); wspush8(b); wspush8(c); wspush8(a); }
void op_jmu() { cpu.rom.ptr = wspop8(); }
void op_jsu() { rspush16(cpu.rom.ptr); cpu.rom.ptr = wspop16(); }
void op_jmc() { if(wspop8()) op_jmu(); }
void op_jsc() { if(wspop8()) op_jsu(); }
void op_equ() { wspush8(wspop8() == wspop8()); }
void op_neq() { wspush8(wspop8() != wspop8()); }
void op_gth() {	wspush8(wspop8() < wspop8()); }
void op_lth() {	wspush8(wspop8() > wspop8()); }
void op_and() {	wspush8(wspop8() & wspop8()); }
void op_ora() {	wspush8(wspop8() | wspop8()); }
void op_rol() { wspush8(wspop8() << 1); }
void op_ror() { wspush8(wspop8() >> 1); }
void op_add() { wspush8(wspop8() + wspop8()); }
void op_sub() { wspush8(wspop8() - wspop8()); }
void op_mul() { wspush8(wspop8() * wspop8()); }
void op_div() { wspush8(wspop8() / wspop8()); }
void op_ldr() { wspush8(cpu.ram.dat[wspop16()]); }
void op_str() { cpu.ram.dat[wspop16()] = wspop8(); }

void (*ops[])(void) = {
	op_brk, op_rts, op_lit, op_drp, op_dup, op_swp, op_ovr, op_rot, 
	op_jmu, op_jsu, op_jmc, op_jsc, op_equ, op_neq, op_gth, op_lth, 
	op_and, op_ora, op_rol, op_ror, op_add, op_sub, op_mul, op_div,
	op_ldr, op_str, op_brk, op_brk, op_brk, op_brk, op_brk, op_brk
};

Uint8 opr[][2] = {
	{0,0}, {0,0}, {0,0}, {1,0}, {0,1}, {1,1}, {0,1}, {3,3},
	{2,0}, {2,0}, {2,0}, {2,0}, {2,1}, {2,1}, {2,1}, {2,1},
	{1,0}, {1,0}, {1,0}, {1,0}, {2,1}, {0,0}, {0,0}, {0,0},
	{2,1}, {3,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
};

/* clang-format on */

void
reset(void)
{
	int i;
	cpu.status = 0x00;
	cpu.counter = 0x00;
	cpu.literal = 0x00;
	cpu.rom.ptr = 0x00;
	cpu.wst.ptr = 0x00;
	cpu.rst.ptr = 0x00;
	for(i = 0; i < 256; i++) {
		cpu.wst.dat[i] = 0x00;
		cpu.rst.dat[i] = 0x00;
	}
}

int
error(char *name)
{
	printf("Error: %s, at 0x%04x\n", name, cpu.counter);
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

int
eval(void)
{
	Uint8 instr = cpu.rom.dat[cpu.rom.ptr++];
	if(cpu.literal > 0) {
		wspush8(instr);
		cpu.literal--;
		return 1;
	}
	if(instr < 32) {
		if(cpu.wst.ptr < opr[instr][0])
			return error("Stack underflow");
		/* TODO stack overflow */
		(*ops[instr])();
	}
	if(instr > 0x10)
		setflag(FLAG_ZERO, 0);
	if(cpu.counter == 128) {
		return error("Reached bounds");
	}
	cpu.counter++;
	return 1;
}

void
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
		getflag(FLAG_ZERO) != 0,
		getflag(FLAG_CARRY) != 0,
		getflag(FLAG_TRAPS) != 0);
	printf("\n");
}

int
main(int argc, char *argv[])
{
	FILE *f;
	if(argc < 2)
		return error("No input.");
	if(!(f = fopen(argv[1], "rb")))
		return error("Missing input.");
	start(f);
	/* print result */
	echos(&cpu.wst, 0x40, "stack");
	echom(&cpu.ram, 0x40, "ram");
	return 0;
}
