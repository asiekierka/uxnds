/*
Copyright (c) 2022 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define CONSOLE_STD 0x1
#define CONSOLE_ARG 0x2
#define CONSOLE_EOA 0x3
#define CONSOLE_END 0x4

int system_load(Uxn *u, char *filename);
void system_inspect(Uxn *u);
int system_error(char *msg, const char *err);
Uint8 system_dei(Uxn *u, Uint8 addr);
void system_deo(Uxn *u, Uint8 *d, Uint8 port);
int console_input(Uxn *u, char c, int type);
void console_deo(Uint8 *d, Uint8 port);
