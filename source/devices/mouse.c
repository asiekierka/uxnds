#include "../uxn.h"
#include "mouse.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

void
mouse_down(Uxn *u, Uint8 *d, Uint8 mask)
{
	d[6] |= mask;
	uxn_eval(u, PEEK2(d));
}

void
mouse_up(Uxn *u, Uint8 *d, Uint8 mask)
{
	d[6] &= (~mask);
	uxn_eval(u, PEEK2(d));
}

void
mouse_pos(Uxn *u, Uint8 *d, Uint16 x, Uint16 y)
{
	POKE2(d + 0x2, x);
	POKE2(d + 0x4, y);
	uxn_eval(u, PEEK2(d));
}

void
mouse_scroll(Uxn *u, Uint8 *d, Uint16 x, Uint16 y)
{
	POKE2(d + 0xa, x);
	POKE2(d + 0xc, -y);
	uxn_eval(u, PEEK2(d));
	POKE2(d + 0xa, 0);
	POKE2(d + 0xc, 0);
}
