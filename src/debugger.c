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

#pragma mark - Core

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 0;
}

void
printstack(Stack *s)
{
	Uint8 x, y;
	for(y = 0; y < 0x08; ++y) {
		for(x = 0; x < 0x08; ++x) {
			Uint8 p = y * 0x08 + x;
			printf(p == s->ptr ? "[%02x]" : " %02x ", s->dat[p]);
		}
		printf("\n");
	}
}

#pragma mark - Devices

Uint8
console_poke(Uxn *u, Uint8 *m, Uint8 b0, Uint8 b1)
{
	switch(b0) {
	case 0x08: printf("%c", b1); break;
	case 0x09: printf("0x%02x\n", b1); break;
	case 0x0b: printf("0x%04x\n", (m[0x0a] << 8) + b1); break;
	}
	fflush(stdout);
	(void)u;
	(void)b0;
	return b1;
}

Uint8
file_poke(Uxn *u, Uint8 *m, Uint8 b0, Uint8 b1)
{
	Uint8 read = b0 == 0xd;
	if(read || b0 == 0xf) {
		char *name = (char *)&u->ram.dat[mempeek16(m, 0x8)];
		Uint16 result = 0, length = mempeek16(m, 0xa);
		Uint16 offset = mempeek16(m, 0x4);
		Uint16 addr = (m[b0 - 1] << 8) | b1;
		FILE *f = fopen(name, read ? "r" : (offset ? "a" : "w"));
		if(f) {
			if(fseek(f, offset, SEEK_SET) != -1 && (result = read ? fread(&m[addr], 1, length, f) : fwrite(&m[addr], 1, length, f)))
				printf("%s %d bytes, at %04x from %s\n", read ? "Loaded" : "Saved", length, addr, name);
			fclose(f);
		}
		mempoke16(m, 0x2, result);
	}
	return b1;
}

Uint8
ppnil(Uxn *u, Uint8 *m, Uint8 b0, Uint8 b1)
{
	(void)u;
	(void)m;
	(void)b0;
	return b1;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	printf("RESET --------\n");
	if(!evaluxn(u, PAGE_PROGRAM))
		return error("Reset", "Failed");
	printstack(&u->wst);
	printf("FRAME --------\n");
	if(!evaluxn(u, PAGE_PROGRAM + 0x08))
		return error("Frame", "Failed");
	printstack(&u->wst);
	return 1;
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

	portuxn(&u, 0x00, "console", console_poke);
	portuxn(&u, 0x01, "empty", ppnil);
	portuxn(&u, 0x02, "empty", ppnil);
	portuxn(&u, 0x03, "empty", ppnil);
	portuxn(&u, 0x04, "empty", ppnil);
	portuxn(&u, 0x05, "empty", ppnil);
	portuxn(&u, 0x06, "file", file_poke);
	start(&u);

	return 0;
}
