#include <stdio.h>
#include "../../include/uxn.h"

#ifndef CPU_ERROR_CHECKING
#define NO_STACK_CHECKS
#endif

/*
Copyright (u) 2021 Devine Lu Linvega
Copyright (u) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define MODE_RETURN 0x40
#define MODE_KEEP 0x80

#pragma mark - Operations

/* clang-format off */
#define devw8(x, y) { u->deo(u, (x), (y)); }
#define devw16(x, y) { u->deo(u, (x), (y) >> 8); u->deo(u, (x) + 1, (y)); }
#define devr8(o, x) { o = u->dei(u, x); }

/* clang-format on */

#pragma mark - Core

ITCM_ARM_CODE
int
uxn_eval(Uxn *u, Uint16 vec)
{
	Uint8 instr;
	if(!vec || u->dev[0x0f])
		return 0;
	u->ram.ptr = vec;
	if(u->wst.ptr > 0xf8) u->wst.ptr = 0xf8;
	while((instr = u->ram.dat[u->ram.ptr++])) {
		switch(instr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-variable"
		case 0x00: /* BRK */
			return 1;
		case 0x20: /* JCI */
			{
				Uint8 b = u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;

				if(!b) { u->ram.ptr += 2; break; }
			}
			// fall through
		case 0x40: /* JMI */
			{
				Uint16 a = peek16(u->ram.dat, u->ram.ptr);
				u->ram.ptr += a + 2;
			}
			break;
		case 0x60: /* JSI */
			{
				u->ram.ptr += 2;
				u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
				u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
				Uint16 a = peek16(u->ram.dat, u->ram.ptr - 2);
				u->ram.ptr += a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x80: /* LIT */
			{
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x01: /* INC */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = a + 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x02: /* POP */
			{
				u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x06: /* DUP */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x03: /* NIP */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x04: /* SWP */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = a;
				u->wst.dat[u->wst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x07: /* OVR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x05: /* OVR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
				u->wst.dat[u->wst.ptr - 3] = b;
				u->wst.dat[u->wst.ptr - 2] = a;
				u->wst.dat[u->wst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x08: /* EQU */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x09: /* NEQ */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x0a: /* GTH */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x0b: /* LTH */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
			}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x0c: /* JMP */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x0d: /* JCN */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x0e: /* JSR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
				u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x0f: /* STH */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x10: /* LDZ */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x11: /* STZ */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x12: /* LDR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x13: /* STR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				poke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x14: /* LDA */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr - 2] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x15: /* STA */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint8 b = u->wst.dat[u->wst.ptr - 3];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x16: /* DEI */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				devr8(u->wst.dat[u->wst.ptr - 1], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x17: /* DEO */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				devw8(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x18: /* ADD */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x19: /* SUB */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1a: /* MUL */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1b: /* DIV */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				if(a == 0) {
					u->wst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->wst.dat[u->wst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1c: /* AND */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1d: /* ORA */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1e: /* EOR */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0x1f: /* SFT */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0xa0: /* LIT2 */
			{
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, u->ram.ptr++);
				u->wst.dat[u->wst.ptr + 1] = peek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x21: /* INC2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr - 2] = (a + 1) >> 8;
				u->wst.dat[u->wst.ptr - 1] = (a + 1) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x22: /* POP2 */
			{
				(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x26: /* DUP2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x23: /* NIP2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				(u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = a >> 8;
				u->wst.dat[u->wst.ptr - 3] = a & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x24: /* SWP2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr - 4] = b;
				u->wst.dat[u->wst.ptr - 3] = a;
				u->wst.dat[u->wst.ptr - 2] = d;
				u->wst.dat[u->wst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x27: /* OVR2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = d;
				u->wst.dat[u->wst.ptr + 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x25: /* OVR2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4], e = u->wst.dat[u->wst.ptr - 5], f = u->wst.dat[u->wst.ptr - 6];
				u->wst.dat[u->wst.ptr - 6] = d;
				u->wst.dat[u->wst.ptr - 5] = c;
				u->wst.dat[u->wst.ptr - 4] = b;
				u->wst.dat[u->wst.ptr - 3] = a;
				u->wst.dat[u->wst.ptr - 2] = f;
				u->wst.dat[u->wst.ptr - 1] = e;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 6, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x28: /* EQU2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x29: /* NEQ2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x2a: /* GTH2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x2b: /* LTH2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x2c: /* JMP2 */
			{
				u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x2d: /* JCN2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x2e: /* JSR2 */
			{
				u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
				u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x2f: /* STH2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x30: /* LDZ2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = peek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x31: /* STZ2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x32: /* LDR2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x33: /* STR2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				poke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x34: /* LDA2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr - 2] = peek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr - 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x35: /* STA2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 4;
			}
			break;
		case 0x36: /* DEI2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				devr8(u->wst.dat[u->wst.ptr - 1], a);
				devr8(u->wst.dat[u->wst.ptr], a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x37: /* DEO2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				devw16(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 3;
			}
			break;
		case 0x38: /* ADD2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = (b + a) >> 8;
				u->wst.dat[u->wst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x39: /* SUB2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = (b - a) >> 8;
				u->wst.dat[u->wst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3a: /* MUL2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr - 4] = (b * a) >> 8;
				u->wst.dat[u->wst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3b: /* DIV2 */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				if(a == 0) {
					u->wst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->wst.dat[u->wst.ptr - 4] = (b / a) >> 8;
				u->wst.dat[u->wst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3c: /* AND2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr - 4] = d & b;
				u->wst.dat[u->wst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3d: /* ORA2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr - 4] = d | b;
				u->wst.dat[u->wst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3e: /* EOR2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr - 4] = d ^ b;
				u->wst.dat[u->wst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 2;
			}
			break;
		case 0x3f: /* SFT2 */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				u->wst.dat[u->wst.ptr - 3] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) >> 8;
				u->wst.dat[u->wst.ptr - 2] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
				u->wst.ptr -= 1;
			}
			break;
		case 0xc0: /* LITr */
			{
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x41: /* INCr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = a + 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x42: /* POPr */
			{
				u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x46: /* DUPr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x43: /* NIPr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x44: /* SWPr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = a;
				u->rst.dat[u->rst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x47: /* OVRr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x45: /* OVRr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
				u->rst.dat[u->rst.ptr - 3] = b;
				u->rst.dat[u->rst.ptr - 2] = a;
				u->rst.dat[u->rst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x48: /* EQUr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x49: /* NEQr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x4a: /* GTHr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x4b: /* LTHr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x4c: /* JMPr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x4d: /* JCNr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x4e: /* JSRr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
				u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x4f: /* STHr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x50: /* LDZr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x51: /* STZr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x52: /* LDRr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x53: /* STRr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				poke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x54: /* LDAr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr - 2] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x55: /* STAr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint8 b = u->rst.dat[u->rst.ptr - 3];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x56: /* DEIr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				devr8(u->rst.dat[u->rst.ptr - 1], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x57: /* DEOr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				devw8(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x58: /* ADDr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x59: /* SUBr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5a: /* MULr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5b: /* DIVr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				if(a == 0) {
					u->rst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->rst.dat[u->rst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5c: /* ANDr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5d: /* ORAr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5e: /* EORr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x5f: /* SFTr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0xe0: /* LIT2r */
			{
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, u->ram.ptr++);
				u->rst.dat[u->rst.ptr + 1] = peek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x61: /* INC2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr - 2] = (a + 1) >> 8;
				u->rst.dat[u->rst.ptr - 1] = (a + 1) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x62: /* POP2r */
			{
				(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x66: /* DUP2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x63: /* NIP2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				(u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = a >> 8;
				u->rst.dat[u->rst.ptr - 3] = a & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x64: /* SWP2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr - 4] = b;
				u->rst.dat[u->rst.ptr - 3] = a;
				u->rst.dat[u->rst.ptr - 2] = d;
				u->rst.dat[u->rst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x67: /* OVR2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = d;
				u->rst.dat[u->rst.ptr + 1] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x65: /* OVR2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4], e = u->rst.dat[u->rst.ptr - 5], f = u->rst.dat[u->rst.ptr - 6];
				u->rst.dat[u->rst.ptr - 6] = d;
				u->rst.dat[u->rst.ptr - 5] = c;
				u->rst.dat[u->rst.ptr - 4] = b;
				u->rst.dat[u->rst.ptr - 3] = a;
				u->rst.dat[u->rst.ptr - 2] = f;
				u->rst.dat[u->rst.ptr - 1] = e;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 6, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x68: /* EQU2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x69: /* NEQ2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x6a: /* GTH2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x6b: /* LTH2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x6c: /* JMP2r */
			{
				u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x6d: /* JCN2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x6e: /* JSR2r */
			{
				u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
				u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x6f: /* STH2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x70: /* LDZ2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = peek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x71: /* STZ2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x72: /* LDR2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x73: /* STR2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				poke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x74: /* LDA2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr - 2] = peek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr - 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x75: /* STA2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 4;
			}
			break;
		case 0x76: /* DEI2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				devr8(u->rst.dat[u->rst.ptr - 1], a);
				devr8(u->rst.dat[u->rst.ptr], a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x77: /* DEO2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				devw16(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 3;
			}
			break;
		case 0x78: /* ADD2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = (b + a) >> 8;
				u->rst.dat[u->rst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x79: /* SUB2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = (b - a) >> 8;
				u->rst.dat[u->rst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7a: /* MUL2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr - 4] = (b * a) >> 8;
				u->rst.dat[u->rst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7b: /* DIV2r */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				if(a == 0) {
					u->rst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->rst.dat[u->rst.ptr - 4] = (b / a) >> 8;
				u->rst.dat[u->rst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7c: /* AND2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr - 4] = d & b;
				u->rst.dat[u->rst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7d: /* ORA2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr - 4] = d | b;
				u->rst.dat[u->rst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7e: /* EOR2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr - 4] = d ^ b;
				u->rst.dat[u->rst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 2;
			}
			break;
		case 0x7f: /* SFT2r */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				u->rst.dat[u->rst.ptr - 3] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) >> 8;
				u->rst.dat[u->rst.ptr - 2] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
				u->rst.ptr -= 1;
			}
			break;
		case 0x81: /* INCk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = a + 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x82: /* POPk */
			{
				u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x86: /* DUPk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = a;
				u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x83: /* NIPk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x84: /* SWPk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = a;
				u->wst.dat[u->wst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x87: /* OVRk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
				u->wst.dat[u->wst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 252, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 3;
			}
			break;
		case 0x85: /* OVRk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
				u->wst.dat[u->wst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 252, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 3;
			}
			break;
		case 0x88: /* EQUk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x89: /* NEQk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x8a: /* GTHk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x8b: /* LTHk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x8c: /* JMPk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x8d: /* JCNk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x8e: /* JSRk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
				u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x8f: /* STHk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x90: /* LDZk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x91: /* STZk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x92: /* LDRk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x93: /* STRk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				poke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x94: /* LDAk */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x95: /* STAk */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint8 b = u->wst.dat[u->wst.ptr - 3];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x96: /* DEIk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				devr8(u->wst.dat[u->wst.ptr], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x97: /* DEOk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				devw8(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x98: /* ADDk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x99: /* SUBk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9a: /* MULk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9b: /* DIVk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				if(a == 0) {
					u->wst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->wst.dat[u->wst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9c: /* ANDk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9d: /* ORAk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9e: /* EORk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x9f: /* SFTk */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xa1: /* INC2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr] = (a + 1) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (a + 1) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xa2: /* POP2k */
			{
				(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xa6: /* DUP2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
				u->wst.dat[u->wst.ptr + 2] = b;
				u->wst.dat[u->wst.ptr + 3] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 251, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 4;
			}
			break;
		case 0xa3: /* NIP2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				(u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = a >> 8;
				u->wst.dat[u->wst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xa4: /* SWP2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
				u->wst.dat[u->wst.ptr + 2] = d;
				u->wst.dat[u->wst.ptr + 3] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 251, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 4;
			}
			break;
		case 0xa7: /* OVR2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = d;
				u->wst.dat[u->wst.ptr + 1] = c;
				u->wst.dat[u->wst.ptr + 2] = b;
				u->wst.dat[u->wst.ptr + 3] = a;
				u->wst.dat[u->wst.ptr + 4] = d;
				u->wst.dat[u->wst.ptr + 5] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 249, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 6;
			}
			break;
		case 0xa5: /* OVR2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4], e = u->wst.dat[u->wst.ptr - 5], f = u->wst.dat[u->wst.ptr - 6];
				u->wst.dat[u->wst.ptr] = d;
				u->wst.dat[u->wst.ptr + 1] = c;
				u->wst.dat[u->wst.ptr + 2] = b;
				u->wst.dat[u->wst.ptr + 3] = a;
				u->wst.dat[u->wst.ptr + 4] = f;
				u->wst.dat[u->wst.ptr + 5] = e;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 6, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 249, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 6;
			}
			break;
		case 0xa8: /* EQU2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xa9: /* NEQ2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xaa: /* GTH2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xab: /* LTH2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xac: /* JMP2k */
			{
				u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xad: /* JCN2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xae: /* JSR2k */
			{
				u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
				u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xaf: /* STH2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xb0: /* LDZ2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr + 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xb1: /* STZ2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb2: /* LDR2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->wst.dat[u->wst.ptr + 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xb3: /* STR2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				poke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb4: /* LDA2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr] = peek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr + 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xb5: /* STA2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb6: /* DEI2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				devr8(u->wst.dat[u->wst.ptr], a);
				devr8(u->wst.dat[u->wst.ptr + 1], a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xb7: /* DEO2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				devw16(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb8: /* ADD2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = (b + a) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xb9: /* SUB2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = (b - a) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xba: /* MUL2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				u->wst.dat[u->wst.ptr] = (b * a) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xbb: /* DIV2k */
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				if(a == 0) {
					u->wst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->wst.dat[u->wst.ptr] = (b / a) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xbc: /* AND2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = d & b;
				u->wst.dat[u->wst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xbd: /* ORA2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = d | b;
				u->wst.dat[u->wst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xbe: /* EOR2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
				u->wst.dat[u->wst.ptr] = d ^ b;
				u->wst.dat[u->wst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xbf: /* SFT2k */
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				u->wst.dat[u->wst.ptr] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) >> 8;
				u->wst.dat[u->wst.ptr + 1] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xc1: /* INCkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = a + 1;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xc2: /* POPkr */
			{
				u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xc6: /* DUPkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = a;
				u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xc3: /* NIPkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xc4: /* SWPkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = a;
				u->rst.dat[u->rst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xc7: /* OVRkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
				u->rst.dat[u->rst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 252, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 3;
			}
			break;
		case 0xc5: /* OVRkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
				u->rst.dat[u->rst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 252, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 3;
			}
			break;
		case 0xc8: /* EQUkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xc9: /* NEQkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xca: /* GTHkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xcb: /* LTHkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xcc: /* JMPkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xcd: /* JCNkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xce: /* JSRkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
				u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xcf: /* STHkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0xd0: /* LDZkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xd1: /* STZkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd2: /* LDRkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xd3: /* STRkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				poke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd4: /* LDAkr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xd5: /* STAkr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint8 b = u->rst.dat[u->rst.ptr - 3];
				poke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd6: /* DEIkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				devr8(u->rst.dat[u->rst.ptr], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xd7: /* DEOkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				devw8(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd8: /* ADDkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xd9: /* SUBkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xda: /* MULkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xdb: /* DIVkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				if(a == 0) {
					u->rst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->rst.dat[u->rst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xdc: /* ANDkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xdd: /* ORAkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xde: /* EORkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xdf: /* SFTkr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xe1: /* INC2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr] = (a + 1) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (a + 1) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xe2: /* POP2kr */
			{
				(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xe6: /* DUP2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
				u->rst.dat[u->rst.ptr + 2] = b;
				u->rst.dat[u->rst.ptr + 3] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 251, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 4;
			}
			break;
		case 0xe3: /* NIP2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				(u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = a >> 8;
				u->rst.dat[u->rst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xe4: /* SWP2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = b;
				u->rst.dat[u->rst.ptr + 1] = a;
				u->rst.dat[u->rst.ptr + 2] = d;
				u->rst.dat[u->rst.ptr + 3] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 251, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 4;
			}
			break;
		case 0xe7: /* OVR2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = d;
				u->rst.dat[u->rst.ptr + 1] = c;
				u->rst.dat[u->rst.ptr + 2] = b;
				u->rst.dat[u->rst.ptr + 3] = a;
				u->rst.dat[u->rst.ptr + 4] = d;
				u->rst.dat[u->rst.ptr + 5] = c;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 249, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 6;
			}
			break;
		case 0xe5: /* OVR2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4], e = u->rst.dat[u->rst.ptr - 5], f = u->rst.dat[u->rst.ptr - 6];
				u->rst.dat[u->rst.ptr] = d;
				u->rst.dat[u->rst.ptr + 1] = c;
				u->rst.dat[u->rst.ptr + 2] = b;
				u->rst.dat[u->rst.ptr + 3] = a;
				u->rst.dat[u->rst.ptr + 4] = f;
				u->rst.dat[u->rst.ptr + 5] = e;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 6, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 249, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 6;
			}
			break;
		case 0xe8: /* EQU2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xe9: /* NEQ2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xea: /* GTH2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xeb: /* LTH2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0xec: /* JMP2kr */
			{
				u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xed: /* JCN2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xee: /* JSR2kr */
			{
				u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
				u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
				u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xef: /* STH2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				u->wst.dat[u->wst.ptr] = b;
				u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0xf0: /* LDZ2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr + 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xf1: /* STZ2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf2: /* LDR2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->rst.dat[u->rst.ptr + 1] = peek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xf3: /* STR2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				poke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf4: /* LDA2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr] = peek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr + 1] = peek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xf5: /* STA2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				poke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf6: /* DEI2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				devr8(u->rst.dat[u->rst.ptr], a);
				devr8(u->rst.dat[u->rst.ptr + 1], a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xf7: /* DEO2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				devw16(a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf8: /* ADD2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = (b + a) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xf9: /* SUB2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = (b - a) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xfa: /* MUL2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				u->rst.dat[u->rst.ptr] = (b * a) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xfb: /* DIV2kr */
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				if(a == 0) {
					u->rst.error = 3;
#ifndef NO_STACK_CHECKS
					goto error;
#endif
					a = 1;
				}
				u->rst.dat[u->rst.ptr] = (b / a) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xfc: /* AND2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = d & b;
				u->rst.dat[u->rst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xfd: /* ORA2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = d | b;
				u->rst.dat[u->rst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xfe: /* EOR2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
				u->rst.dat[u->rst.ptr] = d ^ b;
				u->rst.dat[u->rst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0xff: /* SFT2kr */
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				u->rst.dat[u->rst.ptr] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) >> 8;
				u->rst.dat[u->rst.ptr + 1] = (b >> (a & 0x0f) << ((a & 0xf0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
#pragma GCC diagnostic pop
		}
	}
	return 1;
#ifndef NO_STACK_CHECKS
error:
	dprintf("Halted: %s-stack %sflow#%04x, at 0x%04x\n",
		u->wst.error ? "Working" : "Return",
		((u->wst.error | u->rst.error) & 2) ? "over" : "under",
		instr,
		u->ram.ptr);
	return 0;
/*	if(u->wst.error)
		return uxn_halt(u, u->wst.error, "Working-stack", instr);
	else
		return uxn_halt(u, u->rst.error, "Return-stack", instr); */
#endif
}

int
resetuxn(Uxn *u)
{
	// Reset the stacks
	memset(&(u->wst), 0, sizeof(Stack));
	memset(&(u->rst), 0, sizeof(Stack));
	memset(u->dev, 0, 16 * 16);

	// Reset RAM
	memset(u->ram.dat, 0, 65536 * RAM_PAGES);
	return 1;
}

int
uxn_boot(Uxn *u, Uint8 *ram, Dei *dei, Deo *deo)
{
        memset(u, 0, sizeof(*u));
        u->ram.dat = ram;
	u->dei = dei;
	u->deo = deo;
	return resetuxn(u);
}
