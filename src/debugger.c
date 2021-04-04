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
console_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	switch(b0) {
	case 0x08: printf("%c", b1); break;
	case 0x09: printf("0x%02x\n", b1); break;
	case 0x0b: printf("0x%04x\n", (m[ptr + 0x0a] << 8) + b1); break;
	}
	fflush(stdout);
	(void)m;
	(void)ptr;
	(void)b0;
	return b1;
}

Uint8
file_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	Uint8 *m = u->ram.dat;
	char *name = (char *)&m[(m[ptr + 8] << 8) + m[ptr + 8 + 1]];
	Uint16 length = (m[ptr + 8 + 2] << 8) + m[ptr + 8 + 3];
	Uint16 offset = (m[ptr + 0] << 8) + m[ptr + 1];
	if(b0 == 0x0d) {
		Uint16 addr = (m[ptr + 8 + 4] << 8) + b1;
		FILE *f = fopen(name, "r");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fread(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Loaded %d bytes, at %04x from %s\n", length, addr, name);
		}
	} else if(b0 == 0x0f) {
		Uint16 addr = (m[ptr + 8 + 6] << 8) + b1;
		FILE *f = fopen(name, (m[ptr + 2] & 0x1) ? "a" : "w");
		if(f && fseek(f, offset, SEEK_SET) != -1 && fwrite(&m[addr], length, 1, f)) {
			fclose(f);
			printf("Saved %d bytes, at %04x from %s\n", length, addr, name);
		}
	}
	return b1;
}

Uint8
ppnil(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	(void)u;
	(void)ptr;
	(void)b0;
	return b1;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	printf("RESET --------\n");
	if(!evaluxn(u, PAGE_VECTORS))
		return error("Reset", "Failed");
	printstack(&u->wst);
	printf("FRAME --------\n");
	if(!evaluxn(u, PAGE_VECTORS + 0x08))
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
