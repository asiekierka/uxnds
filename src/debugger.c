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

void
console_talk(Device *d, Uint8 b0, Uint8 w)
{
	if(!w) return;
	switch(b0) {
	case 0x8: printf("%c", d->dat[0x8]); break;
	case 0x9: printf("0x%02x", d->dat[0x9]); break;
	case 0xb: printf("0x%04x", mempeek16(d->dat, 0xa)); break;
	case 0xd: printf("%s", &d->mem[mempeek16(d->dat, 0xc)]); break;
	}
	fflush(stdout);
}

void
file_talk(Device *d, Uint8 b0, Uint8 w)
{
	Uint8 read = b0 == 0xd;
	if(w && (read || b0 == 0xf)) {
		char *name = (char *)&d->mem[mempeek16(d->dat, 0x8)];
		Uint16 result = 0, length = mempeek16(d->dat, 0xa);
		Uint16 offset = mempeek16(d->dat, 0x4);
		Uint16 addr = mempeek16(d->dat, b0 - 1);
		FILE *f = fopen(name, read ? "r" : (offset ? "a" : "w"));
		if(f) {
			if(fseek(f, offset, SEEK_SET) != -1 && (result = read ? fread(&d->mem[addr], 1, length, f) : fwrite(&d->mem[addr], 1, length, f)))
				printf("%s %d bytes, at %04x from %s\n", read ? "Loaded" : "Saved", result, addr, name);
			fclose(f);
		}
		mempoke16(d->dat, 0x2, result);
	}
}

void
nil_talk(Device *d, Uint8 b0, Uint8 w)
{
	(void)d;
	(void)b0;
	(void)w;
}

#pragma mark - Generics

int
start(Uxn *u)
{
	printf("RESET --------\n");
	if(!evaluxn(u, PAGE_PROGRAM))
		return error("Reset", "Failed");
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

	portuxn(&u, 0x00, "empty", nil_talk);
	portuxn(&u, 0x01, "console", console_talk);
	portuxn(&u, 0x02, "empty", nil_talk);
	portuxn(&u, 0x03, "empty", nil_talk);
	portuxn(&u, 0x04, "empty", nil_talk);
	portuxn(&u, 0x05, "empty", nil_talk);
	portuxn(&u, 0x06, "empty", nil_talk);
	portuxn(&u, 0x07, "empty", nil_talk);
	portuxn(&u, 0x08, "empty", nil_talk);
	portuxn(&u, 0x09, "empty", nil_talk);
	portuxn(&u, 0x0a, "file", file_talk);
	portuxn(&u, 0x0b, "empty", nil_talk);
	portuxn(&u, 0x0c, "empty", nil_talk);
	portuxn(&u, 0x0d, "empty", nil_talk);
	portuxn(&u, 0x0e, "empty", nil_talk);
	portuxn(&u, 0x0f, "empty", nil_talk);
	start(&u);

	return 0;
}
