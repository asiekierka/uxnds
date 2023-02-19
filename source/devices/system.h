/*
Copyright (c) 2022 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define PEEK16(d) ((d)[0] << 8 | (d)[1])

int system_load(Uxn *u, char *filename);
void system_deo(Uxn *u, Uint8 *d, Uint8 port);
void system_inspect(Uxn *u);
