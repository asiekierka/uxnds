#include <stdio.h>
#include "cpu.h"

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

int
main(int argc, char *argv[])
{
	FILE *f;
	Computer cpu;
	if(argc < 2)
		return error("No input.", 0);
	if(!(f = fopen(argv[1], "rb")))
		return error("Missing input.", 0);
	if(!load(f))
		return error("Load error", 0);
	if(!boot())
		return error("Boot error", 0);
	/* print result */
	echos(&cpu.wst, 0x40, "stack");
	echom(&cpu.ram, 0x40, "ram");
	echof();
	return 0;
}
