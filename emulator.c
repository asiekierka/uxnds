#include <stdio.h>
#include "uxn.h"

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

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

void
echof(Uxn *c)
{
	printf("ended @ %d steps | hf: %x sf: %x sf: %x cf: %x\n",
		c->counter,
		getflag(&c->status, FLAG_HALT) != 0,
		getflag(&c->status, FLAG_SHORT) != 0,
		getflag(&c->status, FLAG_SIGN) != 0,
		getflag(&c->status, FLAG_COND) != 0);
}

int
main(int argc, char *argv[])
{
	Uxn cpu;
	if(argc < 2)
		return error(&cpu, "No input.", 0);
	if(!load(&cpu, argv[1]))
		return error(&cpu, "Load error", 0);
	if(!boot(&cpu))
		return error(&cpu, "Boot error", 0);
	/* print result */
	echos(&cpu.wst, 0x40, "stack");
	echom(&cpu.ram, 0x40, "ram");
	echof(&cpu);
	return 0;
}
