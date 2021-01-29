#include <stdio.h>

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef unsigned char Uint8;
#define STACK_DEPTH 256

Uint8 data[STACK_DEPTH];
Uint8 address[STACK_DEPTH];

void
stackprint(Uint8 *stack)
{
	int i;
	for(i = 0; i < STACK_DEPTH; ++i) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", stack[i]);
	}
	printf("\n");
}

int
main(int argc, char *argv[])
{
	printf("hello\n");
	stackprint(data);
	return 0;
}
