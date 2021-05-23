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

/* clang-format off */
static inline void   devpoke8(Device *d, Uint8 a, Uint8 b) { d->dat[a & 0xf] = b; d->talk(d, a & 0x0f, 1); }
static inline Uint8  devpeek8(Device *d, Uint8 a) { d->talk(d, a & 0x0f, 0); return d->dat[a & 0xf];  }
static inline void   devpoke16(Device *d, Uint8 a, Uint16 b) { devpoke8(d, a, b >> 8); devpoke8(d, a + 1, b); }
static inline Uint16 devpeek16(Device *d, Uint16 a) { return (devpeek8(d, a) << 8) + devpeek8(d, a + 1); }
/* clang-format on */

ITCM_ARM_CODE
int
evaluxn(Uxn *u, Uint16 vec)
{
	Uint8 instr;
	u->ram.ptr = vec;
	while(u->ram.ptr) {
		instr = u->ram.dat[u->ram.ptr++];
		switch(instr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-variable"
		case 0x00: /* BRK */
		case 0x20: /* BRK2 */
		case 0x40: /* BRKr */
		case 0x60: /* BRK2r */
		case 0x80: /* BRKk */
		case 0xa0: /* BRK2k */
		case 0xc0: /* BRKkr */
		case 0xe0: /* BRK2kr */
			__asm__( "evaluxn_00_BRK:" );
			{
				u->ram.ptr = 0;
			}
			break;
		case 0x01: /* LIT */
		case 0x81: /* LITk */
			__asm__( "evaluxn_01_LIT:" );
			{
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 254, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 1;
			}
			break;
		case 0x02: /* NOP */
		case 0x22: /* NOP2 */
		case 0x42: /* NOPr */
		case 0x62: /* NOP2r */
		case 0x82: /* NOPk */
		case 0xa2: /* NOP2k */
		case 0xc2: /* NOPkr */
		case 0xe2: /* NOP2kr */
			__asm__( "evaluxn_02_NOP:" );
			{
				(void)u;
			}
			break;
		case 0x03: /* POP */
			__asm__( "evaluxn_03_POP:" );
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
		case 0x04: /* DUP */
			__asm__( "evaluxn_04_DUP:" );
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
		case 0x05: /* SWP */
			__asm__( "evaluxn_05_SWP:" );
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
		case 0x06: /* OVR */
			__asm__( "evaluxn_06_OVR:" );
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
		case 0x07: /* ROT */
			__asm__( "evaluxn_07_ROT:" );
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
			__asm__( "evaluxn_08_EQU:" );
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
			__asm__( "evaluxn_09_NEQ:" );
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
			__asm__( "evaluxn_0a_GTH:" );
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
			__asm__( "evaluxn_0b_LTH:" );
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
			__asm__( "evaluxn_0c_JMP:" );
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
			__asm__( "evaluxn_0d_JCN:" );
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
			__asm__( "evaluxn_0e_JSR:" );
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
			__asm__( "evaluxn_0f_STH:" );
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
			__asm__( "evaluxn_10_LDZ:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x11: /* STZ */
			__asm__( "evaluxn_11_STZ:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				mempoke8(u->ram.dat, a, b);
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
			__asm__( "evaluxn_12_LDR:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x13: /* STR */
			__asm__( "evaluxn_13_STR:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
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
			__asm__( "evaluxn_14_LDA:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_15_STA:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint8 b = u->wst.dat[u->wst.ptr - 3];
				mempoke8(u->ram.dat, a, b);
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
			__asm__( "evaluxn_16_DEI:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 1, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x17: /* DEO */
			__asm__( "evaluxn_17_DEO:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				devpoke8(&u->dev[a >> 4], a, b);
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
			__asm__( "evaluxn_18_ADD:" );
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
			__asm__( "evaluxn_19_SUB:" );
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
			__asm__( "evaluxn_1a_MUL:" );
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
			__asm__( "evaluxn_1b_DIV:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
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
			__asm__( "evaluxn_1c_AND:" );
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
			__asm__( "evaluxn_1d_ORA:" );
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
			__asm__( "evaluxn_1e_EOR:" );
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
			__asm__( "evaluxn_1f_SFT:" );
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
		case 0x21: /* LIT2 */
		case 0xa1: /* LIT2k */
			__asm__( "evaluxn_21_LIT2:" );
			{
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
				u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr > 253, 0)) {
					u->wst.error = 2;
					goto error;
				}
#endif
				u->wst.ptr += 2;
			}
			break;
		case 0x23: /* POP2 */
			__asm__( "evaluxn_23_POP2:" );
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
		case 0x24: /* DUP2 */
			__asm__( "evaluxn_24_DUP2:" );
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
		case 0x25: /* SWP2 */
			__asm__( "evaluxn_25_SWP2:" );
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
		case 0x26: /* OVR2 */
			__asm__( "evaluxn_26_OVR2:" );
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
		case 0x27: /* ROT2 */
			__asm__( "evaluxn_27_ROT2:" );
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
			__asm__( "evaluxn_28_EQU2:" );
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
			__asm__( "evaluxn_29_NEQ2:" );
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
			__asm__( "evaluxn_2a_GTH2:" );
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
			__asm__( "evaluxn_2b_LTH2:" );
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
			__asm__( "evaluxn_2c_JMP2:" );
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
			__asm__( "evaluxn_2d_JCN2:" );
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
			__asm__( "evaluxn_2e_JSR2:" );
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
			__asm__( "evaluxn_2f_STH2:" );
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
			__asm__( "evaluxn_30_LDZ2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_31_STZ2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				mempoke16(u->ram.dat, a, b);
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
			__asm__( "evaluxn_32_LDR2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
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
			__asm__( "evaluxn_33_STR2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
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
			__asm__( "evaluxn_34_LDA2:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x35: /* STA2 */
			__asm__( "evaluxn_35_STA2:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				mempoke16(u->ram.dat, a, b);
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
			__asm__( "evaluxn_36_DEI2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
				u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
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
			__asm__( "evaluxn_37_DEO2:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				devpoke16(&u->dev[a >> 4], a, b);
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
			__asm__( "evaluxn_38_ADD2:" );
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
			__asm__( "evaluxn_39_SUB2:" );
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
			__asm__( "evaluxn_3a_MUL2:" );
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
			__asm__( "evaluxn_3b_DIV2:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
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
			__asm__( "evaluxn_3c_AND2:" );
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
			__asm__( "evaluxn_3d_ORA2:" );
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
			__asm__( "evaluxn_3e_EOR2:" );
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
			__asm__( "evaluxn_3f_SFT2:" );
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
		case 0x41: /* LITr */
		case 0xc1: /* LITkr */
			__asm__( "evaluxn_41_LITr:" );
			{
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 254, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 1;
			}
			break;
		case 0x43: /* POPr */
			__asm__( "evaluxn_43_POPr:" );
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
		case 0x44: /* DUPr */
			__asm__( "evaluxn_44_DUPr:" );
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
		case 0x45: /* SWPr */
			__asm__( "evaluxn_45_SWPr:" );
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
		case 0x46: /* OVRr */
			__asm__( "evaluxn_46_OVRr:" );
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
		case 0x47: /* ROTr */
			__asm__( "evaluxn_47_ROTr:" );
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
			__asm__( "evaluxn_48_EQUr:" );
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
			__asm__( "evaluxn_49_NEQr:" );
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
			__asm__( "evaluxn_4a_GTHr:" );
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
			__asm__( "evaluxn_4b_LTHr:" );
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
			__asm__( "evaluxn_4c_JMPr:" );
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
			__asm__( "evaluxn_4d_JCNr:" );
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
			__asm__( "evaluxn_4e_JSRr:" );
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
			__asm__( "evaluxn_4f_STHr:" );
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
			__asm__( "evaluxn_50_LDZr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x51: /* STZr */
			__asm__( "evaluxn_51_STZr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				mempoke8(u->ram.dat, a, b);
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
			__asm__( "evaluxn_52_LDRr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x53: /* STRr */
			__asm__( "evaluxn_53_STRr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
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
			__asm__( "evaluxn_54_LDAr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_55_STAr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint8 b = u->rst.dat[u->rst.ptr - 3];
				mempoke8(u->ram.dat, a, b);
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
			__asm__( "evaluxn_56_DEIr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 1, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x57: /* DEOr */
			__asm__( "evaluxn_57_DEOr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				devpoke8(&u->dev[a >> 4], a, b);
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
			__asm__( "evaluxn_58_ADDr:" );
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
			__asm__( "evaluxn_59_SUBr:" );
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
			__asm__( "evaluxn_5a_MULr:" );
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
			__asm__( "evaluxn_5b_DIVr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
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
			__asm__( "evaluxn_5c_ANDr:" );
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
			__asm__( "evaluxn_5d_ORAr:" );
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
			__asm__( "evaluxn_5e_EORr:" );
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
			__asm__( "evaluxn_5f_SFTr:" );
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
		case 0x61: /* LIT2r */
		case 0xe1: /* LIT2kr */
			__asm__( "evaluxn_61_LIT2r:" );
			{
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
				u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr > 253, 0)) {
					u->rst.error = 2;
					goto error;
				}
#endif
				u->rst.ptr += 2;
			}
			break;
		case 0x63: /* POP2r */
			__asm__( "evaluxn_63_POP2r:" );
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
		case 0x64: /* DUP2r */
			__asm__( "evaluxn_64_DUP2r:" );
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
		case 0x65: /* SWP2r */
			__asm__( "evaluxn_65_SWP2r:" );
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
		case 0x66: /* OVR2r */
			__asm__( "evaluxn_66_OVR2r:" );
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
		case 0x67: /* ROT2r */
			__asm__( "evaluxn_67_ROT2r:" );
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
			__asm__( "evaluxn_68_EQU2r:" );
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
			__asm__( "evaluxn_69_NEQ2r:" );
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
			__asm__( "evaluxn_6a_GTH2r:" );
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
			__asm__( "evaluxn_6b_LTH2r:" );
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
			__asm__( "evaluxn_6c_JMP2r:" );
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
			__asm__( "evaluxn_6d_JCN2r:" );
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
			__asm__( "evaluxn_6e_JSR2r:" );
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
			__asm__( "evaluxn_6f_STH2r:" );
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
			__asm__( "evaluxn_70_LDZ2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_71_STZ2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				mempoke16(u->ram.dat, a, b);
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
			__asm__( "evaluxn_72_LDR2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
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
			__asm__( "evaluxn_73_STR2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
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
			__asm__( "evaluxn_74_LDA2r:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x75: /* STA2r */
			__asm__( "evaluxn_75_STA2r:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				mempoke16(u->ram.dat, a, b);
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
			__asm__( "evaluxn_76_DEI2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
				u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
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
			__asm__( "evaluxn_77_DEO2r:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				devpoke16(&u->dev[a >> 4], a, b);
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
			__asm__( "evaluxn_78_ADD2r:" );
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
			__asm__( "evaluxn_79_SUB2r:" );
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
			__asm__( "evaluxn_7a_MUL2r:" );
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
			__asm__( "evaluxn_7b_DIV2r:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
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
			__asm__( "evaluxn_7c_AND2r:" );
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
			__asm__( "evaluxn_7d_ORA2r:" );
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
			__asm__( "evaluxn_7e_EOR2r:" );
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
			__asm__( "evaluxn_7f_SFT2r:" );
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
		case 0x83: /* POPk */
			__asm__( "evaluxn_83_POPk:" );
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
		case 0x84: /* DUPk */
			__asm__( "evaluxn_84_DUPk:" );
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
		case 0x85: /* SWPk */
			__asm__( "evaluxn_85_SWPk:" );
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
		case 0x86: /* OVRk */
			__asm__( "evaluxn_86_OVRk:" );
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
		case 0x87: /* ROTk */
			__asm__( "evaluxn_87_ROTk:" );
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
			__asm__( "evaluxn_88_EQUk:" );
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
			__asm__( "evaluxn_89_NEQk:" );
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
			__asm__( "evaluxn_8a_GTHk:" );
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
			__asm__( "evaluxn_8b_LTHk:" );
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
			__asm__( "evaluxn_8c_JMPk:" );
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
			__asm__( "evaluxn_8d_JCNk:" );
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
			__asm__( "evaluxn_8e_JSRk:" );
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
			__asm__( "evaluxn_8f_STHk:" );
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
			__asm__( "evaluxn_90_LDZk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_91_STZk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x92: /* LDRk */
			__asm__( "evaluxn_92_LDRk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
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
			__asm__( "evaluxn_93_STRk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint8 b = u->wst.dat[u->wst.ptr - 2];
				mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x94: /* LDAk */
			__asm__( "evaluxn_94_LDAk:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_95_STAk:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint8 b = u->wst.dat[u->wst.ptr - 3];
				mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x96: /* DEIk */
			__asm__( "evaluxn_96_DEIk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
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
			__asm__( "evaluxn_97_DEOk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
				devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 2, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0x98: /* ADDk */
			__asm__( "evaluxn_98_ADDk:" );
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
			__asm__( "evaluxn_99_SUBk:" );
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
			__asm__( "evaluxn_9a_MULk:" );
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
			__asm__( "evaluxn_9b_DIVk:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
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
			__asm__( "evaluxn_9c_ANDk:" );
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
			__asm__( "evaluxn_9d_ORAk:" );
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
			__asm__( "evaluxn_9e_EORk:" );
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
			__asm__( "evaluxn_9f_SFTk:" );
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
		case 0xa3: /* POP2k */
			__asm__( "evaluxn_a3_POP2k:" );
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
		case 0xa4: /* DUP2k */
			__asm__( "evaluxn_a4_DUP2k:" );
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
		case 0xa5: /* SWP2k */
			__asm__( "evaluxn_a5_SWP2k:" );
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
		case 0xa6: /* OVR2k */
			__asm__( "evaluxn_a6_OVR2k:" );
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
		case 0xa7: /* ROT2k */
			__asm__( "evaluxn_a7_ROT2k:" );
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
			__asm__( "evaluxn_a8_EQU2k:" );
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
			__asm__( "evaluxn_a9_NEQ2k:" );
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
			__asm__( "evaluxn_aa_GTH2k:" );
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
			__asm__( "evaluxn_ab_LTH2k:" );
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
			__asm__( "evaluxn_ac_JMP2k:" );
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
			__asm__( "evaluxn_ad_JCN2k:" );
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
			__asm__( "evaluxn_ae_JSR2k:" );
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
			__asm__( "evaluxn_af_STH2k:" );
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
			__asm__( "evaluxn_b0_LDZ2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_b1_STZ2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb2: /* LDR2k */
			__asm__( "evaluxn_b2_LDR2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
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
			__asm__( "evaluxn_b3_STR2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb4: /* LDA2k */
			__asm__( "evaluxn_b4_LDA2k:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
				u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_b5_STA2k:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
				Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
				mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 4, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb6: /* DEI2k */
			__asm__( "evaluxn_b6_DEI2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
				u->wst.dat[u->wst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
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
			__asm__( "evaluxn_b7_DEO2k:" );
			{
				Uint8 a = u->wst.dat[u->wst.ptr - 1];
				Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
				devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->wst.ptr < 3, 0)) {
					u->wst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xb8: /* ADD2k */
			__asm__( "evaluxn_b8_ADD2k:" );
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
			__asm__( "evaluxn_b9_SUB2k:" );
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
			__asm__( "evaluxn_ba_MUL2k:" );
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
			__asm__( "evaluxn_bb_DIV2k:" );
			{
				Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
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
			__asm__( "evaluxn_bc_AND2k:" );
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
			__asm__( "evaluxn_bd_ORA2k:" );
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
			__asm__( "evaluxn_be_EOR2k:" );
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
			__asm__( "evaluxn_bf_SFT2k:" );
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
		case 0xc3: /* POPkr */
			__asm__( "evaluxn_c3_POPkr:" );
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
		case 0xc4: /* DUPkr */
			__asm__( "evaluxn_c4_DUPkr:" );
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
		case 0xc5: /* SWPkr */
			__asm__( "evaluxn_c5_SWPkr:" );
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
		case 0xc6: /* OVRkr */
			__asm__( "evaluxn_c6_OVRkr:" );
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
		case 0xc7: /* ROTkr */
			__asm__( "evaluxn_c7_ROTkr:" );
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
			__asm__( "evaluxn_c8_EQUkr:" );
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
			__asm__( "evaluxn_c9_NEQkr:" );
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
			__asm__( "evaluxn_ca_GTHkr:" );
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
			__asm__( "evaluxn_cb_LTHkr:" );
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
			__asm__( "evaluxn_cc_JMPkr:" );
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
			__asm__( "evaluxn_cd_JCNkr:" );
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
			__asm__( "evaluxn_ce_JSRkr:" );
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
			__asm__( "evaluxn_cf_STHkr:" );
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
			__asm__( "evaluxn_d0_LDZkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_d1_STZkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd2: /* LDRkr */
			__asm__( "evaluxn_d2_LDRkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
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
			__asm__( "evaluxn_d3_STRkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint8 b = u->rst.dat[u->rst.ptr - 2];
				mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd4: /* LDAkr */
			__asm__( "evaluxn_d4_LDAkr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
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
			__asm__( "evaluxn_d5_STAkr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint8 b = u->rst.dat[u->rst.ptr - 3];
				mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd6: /* DEIkr */
			__asm__( "evaluxn_d6_DEIkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
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
			__asm__( "evaluxn_d7_DEOkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
				devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 2, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xd8: /* ADDkr */
			__asm__( "evaluxn_d8_ADDkr:" );
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
			__asm__( "evaluxn_d9_SUBkr:" );
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
			__asm__( "evaluxn_da_MULkr:" );
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
			__asm__( "evaluxn_db_DIVkr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
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
			__asm__( "evaluxn_dc_ANDkr:" );
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
			__asm__( "evaluxn_dd_ORAkr:" );
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
			__asm__( "evaluxn_de_EORkr:" );
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
			__asm__( "evaluxn_df_SFTkr:" );
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
		case 0xe3: /* POP2kr */
			__asm__( "evaluxn_e3_POP2kr:" );
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
		case 0xe4: /* DUP2kr */
			__asm__( "evaluxn_e4_DUP2kr:" );
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
		case 0xe5: /* SWP2kr */
			__asm__( "evaluxn_e5_SWP2kr:" );
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
		case 0xe6: /* OVR2kr */
			__asm__( "evaluxn_e6_OVR2kr:" );
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
		case 0xe7: /* ROT2kr */
			__asm__( "evaluxn_e7_ROT2kr:" );
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
			__asm__( "evaluxn_e8_EQU2kr:" );
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
			__asm__( "evaluxn_e9_NEQ2kr:" );
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
			__asm__( "evaluxn_ea_GTH2kr:" );
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
			__asm__( "evaluxn_eb_LTH2kr:" );
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
			__asm__( "evaluxn_ec_JMP2kr:" );
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
			__asm__( "evaluxn_ed_JCN2kr:" );
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
			__asm__( "evaluxn_ee_JSR2kr:" );
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
			__asm__( "evaluxn_ef_STH2kr:" );
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
			__asm__( "evaluxn_f0_LDZ2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_f1_STZ2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf2: /* LDR2kr */
			__asm__( "evaluxn_f2_LDR2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
				u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
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
			__asm__( "evaluxn_f3_STR2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf4: /* LDA2kr */
			__asm__( "evaluxn_f4_LDA2kr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
				u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
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
			__asm__( "evaluxn_f5_STA2kr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
				Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
				mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 4, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf6: /* DEI2kr */
			__asm__( "evaluxn_f6_DEI2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
				u->rst.dat[u->rst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
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
			__asm__( "evaluxn_f7_DEO2kr:" );
			{
				Uint8 a = u->rst.dat[u->rst.ptr - 1];
				Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
				devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
				if(__builtin_expect(u->rst.ptr < 3, 0)) {
					u->rst.error = 1;
					goto error;
				}
#endif
			}
			break;
		case 0xf8: /* ADD2kr */
			__asm__( "evaluxn_f8_ADD2kr:" );
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
			__asm__( "evaluxn_f9_SUB2kr:" );
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
			__asm__( "evaluxn_fa_MUL2kr:" );
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
			__asm__( "evaluxn_fb_DIV2kr:" );
			{
				Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
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
			__asm__( "evaluxn_fc_AND2kr:" );
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
			__asm__( "evaluxn_fd_ORA2kr:" );
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
			__asm__( "evaluxn_fe_EOR2kr:" );
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
			__asm__( "evaluxn_ff_SFT2kr:" );
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
#endif
}

int
bootuxn(Uxn *u)
{
	memset(u, 0, sizeof(*u));
	u->ram.dat = malloc(65536);
	memset(u->ram.dat, 0, 65536);
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb"))) {
		dprintf("Halted: Missing input rom.\n");
		return 0;
	}
	fread(u->ram.dat + PAGE_PROGRAM, sizeof(u->ram.dat) - PAGE_PROGRAM, 1, f);
	dprintf("Uxn loaded[%s].\n", filepath);
	return 1;
}

Device *
portuxn(Uxn *u, Uint8 id, char *name, void (*talkfn)(Device *d, Uint8 b0, Uint8 w))
{
	Device *d = &u->dev[id];
	d->addr = id * 0x10;
	d->u = u;
	d->mem = u->ram.dat;
	d->talk = talkfn;
	dprintf("Device added #%02x: %s, at 0x%04x \n", id, name, d->addr);
	return d;
}
