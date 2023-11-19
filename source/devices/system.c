#include <stdio.h>
#include <stdlib.h>

#include "../uxn.h"
#include "system.h"

/*
Copyright (c) 2022-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static const char *errors[] = {
	"underflow",
	"overflow",
	"division by zero"};

static void
system_print(Stack *s, char *name)
{
	Uint8 i;
	iprintf("<%s>", name);
	for(i = 0; i < s->ptr; i++)
		iprintf(" %02x", s->dat[i]);
	if(!i)
		iprintf(" empty");
	iprintf("\n");
}

static void
system_cmd(Uint8 *ram, Uint16 addr)
{
	if(ram[addr] == 0x1) {
		Uint16 i, length = PEEK2(ram + addr + 1);
		Uint16 a_page = PEEK2(ram + addr + 1 + 2), a_addr = PEEK2(ram + addr + 1 + 4);
		Uint16 b_page = PEEK2(ram + addr + 1 + 6), b_addr = PEEK2(ram + addr + 1 + 8);
		int src = (a_page % RAM_PAGES) * 0x10000, dst = (b_page % RAM_PAGES) * 0x10000;
		for(i = 0; i < length; i++)
			ram[dst + (Uint16)(b_addr + i)] = ram[src + (Uint16)(a_addr + i)];
	}
}

int
system_error(char *msg, const char *err)
{
	iprintf("%s: %s\n", msg, err);
	fflush(stderr);
	return 0;
}

int
system_load(Uxn *u, char *filename)
{
	int l, i = 0;
	FILE *f = fopen(filename, "rb");
	if(!f)
		return 0;
	l = fread(&u->ram.dat[PAGE_PROGRAM], 0x10000 - PAGE_PROGRAM, 1, f);
	while(l && ++i < RAM_PAGES)
		l = fread(u->ram.dat + 0x10000 * i, 0x10000, 1, f);
	fclose(f);
	return 1;
}

void
system_inspect(Uxn *u)
{
	system_print(&u->wst, "wst");
	system_print(&u->rst, "rst");
}

/* IO */

Uint8
system_dei(Uxn *u, Uint8 addr)
{
        switch(addr) {
        case 0x4: return u->wst.ptr;
        case 0x5: return u->rst.ptr;
        default: return u->dev[addr];
        }
}

void
system_deo(Uxn *u, Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x3:
		system_cmd(u->ram.dat, PEEK2(d + 2));
		break;
        case 0x4:
                u->wst.ptr = d[4];
                break;
        case 0x5:
                u->rst.ptr = d[5];
                break;
	case 0xe:
		system_inspect(u);
		break;
	}
}

/* Errors */

int
uxn_halt(Uxn *u, Uint8 instr, Uint8 err, Uint16 addr)
{
	Uint8 *d = &u->dev[0];
	Uint16 handler = PEEK2(d);
	if(handler) {
		u->wst.ptr = 4;
		u->wst.dat[0] = addr >> 0x8;
		u->wst.dat[1] = addr & 0xff;
		u->wst.dat[2] = instr;
		u->wst.dat[3] = err;
		return uxn_eval(u, handler);
	} else {
		system_inspect(u);
		iprintf("%s %s, by %02x at 0x%04x.\n", (instr & 0x40) ? "Return-stack" : "Working-stack", errors[err - 1], instr, addr);
	}
	return 0;
}

/* Console */

int
console_input(Uxn *u, char c, int type)
{
	Uint8 *d = &u->dev[0x10];
	d[0x2] = c;
	d[0x7] = type;
	return uxn_eval(u, PEEK2(d));
}

void
console_deo(Uint8 *d, Uint8 port)
{
	switch(port) {
	case 0x8:
	case 0x9:
		fputc(d[port], stdout);
		fflush(stdout);
		return;
	}
}
