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
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct {
	Uint8 literal, status;
	Uint16 counter;
	Stack wst, rst;
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
echo(Stack *s, Uint8 len, char *name)
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

#pragma mark - Operations

/* clang-format off */

void wspush(Uint8 v) { cpu.wst.dat[cpu.wst.ptr++] = v; }
Uint8 wspop(void) { return cpu.wst.dat[--cpu.wst.ptr]; }
Uint16 wspop16(void) { 

	Uint8 a = cpu.wst.dat[--cpu.wst.ptr];
	Uint8 b = cpu.wst.dat[--cpu.wst.ptr];
	return a + (b << 8); 

}
Uint8 wspeek(void) { return cpu.wst.dat[cpu.wst.ptr - 1]; }
void rspush(Uint8 v) { cpu.rst.dat[cpu.rst.ptr++] = v; }
Uint8 rspop(void) { return cpu.rst.dat[--cpu.rst.ptr]; }

void op_brk() { setflag(FLAG_HALT, 1); }
void op_rts() {	cpu.rom.ptr = rspop(); }
void op_lit() { cpu.literal += cpu.rom.dat[cpu.rom.ptr++]; }
void op_drp() { wspop(); }
void op_dup() { wspush(wspeek()); }
void op_swp() { Uint8 b = wspop(), a = wspop(); wspush(b); wspush(a); }
void op_ovr() { wspush(cpu.wst.dat[cpu.wst.ptr - 2]); }
void op_rot() { Uint8 c = wspop(),b = wspop(),a = wspop(); wspush(b); wspush(c); wspush(a); }
void op_jmu() { cpu.rom.ptr = wspop(); }
void op_jsu() { rspush(cpu.rom.ptr); cpu.rom.ptr = wspop(); }
void op_jmc() { if(wspop()) op_jmu(); }
void op_jsc() { if(wspop()) op_jsu(); }
void op_equ() { wspush(wspop() == wspop()); }
void op_neq() { wspush(wspop() != wspop()); }
void op_gth() {	wspush(wspop() < wspop()); }
void op_lth() {	wspush(wspop() > wspop()); }
void op_and() {	wspush(wspop() & wspop()); }
void op_ora() {	wspush(wspop() | wspop()); }
void op_rol() { wspush(wspop() << 1); }
void op_ror() { wspush(wspop() >> 1); }
void op_add() { wspush(wspop() + wspop()); }
void op_sub() { wspush(wspop() - wspop()); }
void op_mul() { wspush(wspop() * wspop()); }
void op_div() { wspush(wspop() / wspop()); }
void op_ldr() {  }
void op_str() { 

	Uint8 b = wspop();
	Uint16 addr = wspop16();
	printf("store: %02x @ %04x\n", b, addr);
}

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
	{2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1}, {2,1},
	{3,1}, {3,1}
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
	printf("Error: %s\n", name);
	return 0;
}

void
load(FILE *f)
{
	fread(cpu.rom.dat, sizeof(cpu.rom.dat), 1, f);
}

int
eval()
{
	Uint8 instr = cpu.rom.dat[cpu.rom.ptr++];
	if(cpu.literal > 0) {
		wspush(instr);
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
		printf("REACHED COUNTER\n");
		return 0;
	}
	cpu.counter++;
	return 1;
}

void
run(void)
{
	while(!(cpu.status & FLAG_HALT) && eval(cpu))
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
	reset();
	load(f);
	run();
	/* print result */
	echo(&cpu.wst, 0x40, "stack");
	return 0;
}
