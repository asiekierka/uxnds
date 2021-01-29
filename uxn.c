#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define STACK_DEPTH 256
#define ECHO 1

typedef unsigned char Uint8;
typedef unsigned char Uint16;

typedef struct {

} Computer;

Uint8 sptr;
Uint8 stack[STACK_DEPTH];
Uint8 address[STACK_DEPTH];
Uint16 memory[STACK_DEPTH];

void
echo(Uint8 *s, Uint8 len, char *name)
{
	int i;
	printf("%s\n", name);
	for(i = 0; i < len; ++i) {
		if(i % 16 == 0)
			printf("\n");
		if(sptr == i)
			printf("[%02x]", s[i]);
		else
			printf(" %02x ", s[i]);
	}
	printf("\n");
}

void
op_push(Uint8 *s, Uint8 v)
{
	s[sptr++] = v;
}

void
op_pop(Uint8 *s)
{
	s[sptr--] = 0x00;
}

void
reset(Computer *cpu)
{
}

int
disk(Computer *cpu, FILE *f)
{
	int i;
	unsigned short buffer[256];
	reset(cpu);
	if(!fread(buffer, sizeof(buffer), 1, f))
		return 0;

	for(i = 0; i < 128; i++) {
		cpu->memory[i * 2] |= (buffer[i] >> 8) & 0xFF;
		cpu->memory[i * 2 + 1] |= buffer[i] & 0xFF;
	}

	return 1;
}

void
run(Computer *cpu, int debug)
{
}

int
main(int argc, char *argv[])
{
	FILE *f;
	Computer cpu;
	if(argc < 2)
		return error("No input.");
	if(!(f = fopen(argv[1], "rb")))
		return error("Missing input.");
	if(!disk(&cpu, f))
		return error("Unreadable input.");
	run(&cpu, ECHO);
	/* print result */
	echo(stack, 0x40, "stack");
	echo(memory, 0x40, "memory");
	return 0;
}
