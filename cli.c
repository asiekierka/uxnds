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

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

Uint8
consoler(Device *d, Uint8 b)
{
	(void)d;
	(void)b;
	return 0;
}

Uint8
consolew(Device *d, Uint8 b)
{
	(void)d;
	if(b)
		printf("%c", b);
	return 0;
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
echom(Memory *m, Uint16 len, char *name)
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
	printf("\nEnded @ %d steps | hf: %x sf: %x sf: %x cf: %x\n",
		c->counter,
		getflag(&c->status, FLAG_HALT) != 0,
		getflag(&c->status, FLAG_SHORT) != 0,
		getflag(&c->status, FLAG_SIGN) != 0,
		getflag(&c->status, FLAG_COND) != 0);
}

int
main(int argc, char **argv)
{
	Uxn u;
	if(argc < 2)
		return error("Input", "Missing");
	if(!bootuxn(&u))
		return error("Boot", "Failed");
	if(!loaduxn(&u, argv[1]))
		return error("Load", "Failed");
	portuxn(&u, "console", consoler, consolew);
	evaluxn(&u, u.vreset);
	evaluxn(&u, u.vframe);

	echos(&u.wst, 0x40, "stack");
	echom(&u.ram, 0x40, "ram");

	echof(&u);
	return 0;
}
