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

#define STACK_DEPTH 256

typedef unsigned char Uint8;

typedef struct {
	Uint8 literal;
	Uint8 status, counter;
	Uint8 memory[STACK_DEPTH];
	Uint8 mptr, sptr;
	Uint8 stack[STACK_DEPTH];
	Uint8 address[STACK_DEPTH];
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
echo(Uint8 *s, Uint8 len, char *name)
{
	int i;
	printf("%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", s[i]);
	}
	printf("\n\n");
}

#pragma mark - Operations

void
op_push(Uint8 *s, Uint8 *ptr, Uint8 v)
{
	s[(*ptr)++] = v;
}

Uint8
op_pop(Uint8 *s, Uint8 *ptr)
{
	return s[--*ptr];
}

void
reset(void)
{
	int i;
	cpu.status = 0x00;
	cpu.counter = 0x00;
	cpu.mptr = 0x00;
	cpu.sptr = 0x00;
	cpu.literal = 0x00;
	for(i = 0; i < 256; i++)
		cpu.stack[i] = 0x00;
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
	fread(cpu.memory, sizeof(cpu.memory), 1, f);
}

void
eval()
{
	Uint8 instr = cpu.memory[cpu.mptr++];
	Uint8 a, b, c;
	if(cpu.literal > 0) {
		printf("push: %02x[%d](%d)\n", instr, cpu.literal, cpu.sptr);
		op_push(cpu.stack, &cpu.sptr, instr);
		cpu.literal--;
		return;
	}
	switch(instr) {
	case 0x0: setflag(FLAG_HALT, 1); break;
	case 0x1: cpu.literal += cpu.memory[cpu.mptr++]; break;
	case 0x2: printf("??\n"); break;
	case 0x3: /* pop */
		op_pop(cpu.stack, &cpu.sptr);
		break;
	case 0x4: /* dup */
		op_push(cpu.stack, &cpu.sptr, cpu.stack[cpu.sptr - 1]);
		break;
	case 0x5: /* swp */
		b = op_pop(cpu.stack, &cpu.sptr);
		a = op_pop(cpu.stack, &cpu.sptr);
		op_push(cpu.stack, &cpu.sptr, b);
		op_push(cpu.stack, &cpu.sptr, a);
		break;
	case 0x6: /* ovr */
		op_push(cpu.stack, &cpu.sptr, cpu.stack[cpu.sptr - 2]);
		break;
	case 0x7: /* rot */
		c = op_pop(cpu.stack, &cpu.sptr);
		b = op_pop(cpu.stack, &cpu.sptr);
		a = op_pop(cpu.stack, &cpu.sptr);
		op_push(cpu.stack, &cpu.sptr, b);
		op_push(cpu.stack, &cpu.sptr, c);
		op_push(cpu.stack, &cpu.sptr, a);
		break;
	default: printf("Unknown instruction: #%02x\n", instr);
	}
}

void
run(void)
{
	int i;
	while((cpu.status & FLAG_HALT) == 0)
		eval(cpu);
	/* debug */
	printf("ended @ %d  |  ", cpu.counter);
	for(i = 0; i < 4; i++)
		printf("%d-", (cpu.status & (1 << i)) != 0);
	printf("\n\n");
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
	echo(cpu.stack, 0x40, "stack");
	echo(cpu.memory, 0x40, "memory");
	return 0;
}
