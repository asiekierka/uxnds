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
system_print(Stack *s, int ptr, char *name)
{
	Uint8 i;
	iprintf("<%s>", name);
	for(i = 0; i < ptr; i++)
		iprintf(" %02x", s->dat[i]);
	if(!i)
		iprintf(" empty");
	iprintf("\n");
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
	system_print(&u->wst, uxn_get_wst_ptr(), "wst");
	system_print(&u->rst, uxn_get_rst_ptr(), "rst");
}

/* IO */

Uint8
system_dei(Uxn *u, Uint8 addr)
{
        switch(addr) {
        case 0x4: return uxn_get_wst_ptr();
        case 0x5: return uxn_get_rst_ptr();
        default: return u->dev[addr];
        }
}

void
system_deo(Uxn *u, Uint8 *d, Uint8 port)
{
	Uint8 *ram;
	Uint16 addr;
	switch(port) {
	case 0x3:
		ram = u->ram.dat;
		addr = PEEK2(d + 2);
		if(ram[addr] == 0x0) {
			Uint8 value = ram[addr + 7];
			Uint16 i, length = PEEK2(ram + addr + 1);
			Uint16 dst_page = PEEK2(ram + addr + 3), dst_addr = PEEK2(ram + addr + 5);
			int dst = (dst_page % RAM_PAGES) * 0x10000;
			for(i = 0; i < length; i++)
				ram[dst + (Uint16)(dst_addr + i)] = value;
		} else if(ram[addr] == 0x1) {
			Uint16 i, length = PEEK2(ram + addr + 1);
			Uint16 a_page = PEEK2(ram + addr + 3), a_addr = PEEK2(ram + addr + 5);
			Uint16 b_page = PEEK2(ram + addr + 7), b_addr = PEEK2(ram + addr + 9);
			int src = (a_page % RAM_PAGES) * 0x10000, dst = (b_page % RAM_PAGES) * 0x10000;
			for(i = 0; i < length; i++)
				ram[dst + (Uint16)(b_addr + i)] = ram[src + (Uint16)(a_addr + i)];
		} else if(ram[addr] == 0x2) {
			Uint16 i, length = PEEK2(ram + addr + 1);
			Uint16 a_page = PEEK2(ram + addr + 3), a_addr = PEEK2(ram + addr + 5);
			Uint16 b_page = PEEK2(ram + addr + 7), b_addr = PEEK2(ram + addr + 9);
			int src = (a_page % RAM_PAGES) * 0x10000, dst = (b_page % RAM_PAGES) * 0x10000;
			for(i = length - 1; i != 0xffff; i--)
				ram[dst + (Uint16)(b_addr + i)] = ram[src + (Uint16)(a_addr + i)];
		} else
			fiprintf(stderr, "Unknown Expansion Command 0x%02x\n", ram[addr]);
		break;
	case 0x4:
		uxn_set_wst_ptr(d[4]);
		break;
	case 0x5:
		uxn_set_rst_ptr(d[5]);
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
		uxn_set_wst_ptr(4);
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
